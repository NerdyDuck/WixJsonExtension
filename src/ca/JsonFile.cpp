
#include "stdafx.h"

using namespace jsoncons;

LPCWSTR vcsJsonFileQuery = L"SELECT `JsonFile`.`JsonFile`, `JsonFile`.`File`, `JsonFile`.`ElementPath`, `JsonFile`.`VerifyPath`, "
L"`JsonFile`.`Name`, `JsonFile`.`Value`, `JsonFile`.`ValueType`, `JsonFile`.`Flags`, `JsonFile`.`Component_` FROM `JsonFile`,`Component`"
L"WHERE `JsonFile`.`Component_`=`Component`.`Component` ORDER BY `File`, `Sequence`";
enum eJsonFileQuery { jfqId = 1, jfqFile, jfqElementPath, jfqVerifyPath, jfqName, jfqValue, jfqValueType, jfqFlags, jfqComponent };

static HRESULT RecursePath(
    __in_z LPCWSTR wzPath,
    __in_z LPCWSTR wzId,
    __in_z LPCWSTR wzComponent,
    __in_z LPCWSTR wzProperty,
    __in int iMode,
    __inout DWORD* pdwCounter,
    __inout MSIHANDLE* phTable,
    __inout MSIHANDLE* phColumns
);


extern "C" UINT WINAPI JsonFile(
    __in MSIHANDLE hInstall
)
{
    //AssertSz(FALSE, "debug JsonFile");

    HRESULT hr = S_OK;
    PMSIHANDLE hView;
    PMSIHANDLE hRec;
    LPWSTR sczId = NULL;
    LPWSTR sczComponent = NULL;
    LPWSTR sczProperty = NULL;
    LPWSTR sczPath = NULL;
    int iSeq = 0;
    DWORD dwCounter = 0;
    MSIHANDLE hTable = NULL;
    MSIHANDLE hColumns = NULL;

    hr = WcaInitialize(hInstall, "JsonFile");
    ExitOnFailure(hr, "Failed to initialize JsonFile.");

    // anything to do?
    if (S_OK != WcaTableExists(L"JsonFile"))
    {
        WcaLog(LOGMSG_STANDARD, "JsonFile table doesn't exist, so there are no .json files to update.");
        ExitFunction();
    }

    // query and loop through all the remove folders exceptions
    hr = WcaOpenExecuteView(vcsJsonFileQuery, &hView);
    ExitOnFailure(hr, "Failed to open view on JsonFile table");

    while (S_OK == (hr = WcaFetchRecord(hView, &hRec)))
    {
        hr = WcaGetRecordString(hRec, jfqId, &sczId);
        ExitOnFailure(hr, "Failed to get remove folder identity.");

        hr = WcaGetRecordString(hRec, jfqComponent, &sczComponent);
        ExitOnFailure(hr, "Failed to get remove folder component.");

        //hr = WcaGetRecordInteger(hRec, jfqSeq, &iSeq);
        //ExitOnFailure(hr, "Failed to get remove folder mode");

        hr = WcaGetProperty(sczProperty, &sczPath);
        ExitOnFailure2(hr, "Failed to resolve remove folder property: %S for row: %S", sczProperty, sczId);

        hr = RecursePath(sczPath, sczId, sczComponent, sczProperty, iSeq, &dwCounter, &hTable, &hColumns);
        ExitOnFailure2(hr, "Failed while navigating path: %S for row: %S", sczPath, sczId);
    }

    // reaching the end of the list is actually a good thing, not an error
    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure occured while processing JsonFile table");

LExit:
    if (hColumns)
    {
        ::MsiCloseHandle(hColumns);
    }

    if (hTable)
    {
        ::MsiCloseHandle(hTable);
    }

    ReleaseStr(sczPath);
    ReleaseStr(sczProperty);
    ReleaseStr(sczComponent);
    ReleaseStr(sczId);

    DWORD er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


static HRESULT RecursePath(
    __in_z LPCWSTR wzPath,
    __in_z LPCWSTR wzId,
    __in_z LPCWSTR wzComponent,
    __in_z LPCWSTR wzProperty,
    __in int iMode,
    __inout DWORD* pdwCounter,
    __inout MSIHANDLE* phTable,
    __inout MSIHANDLE* phColumns
)
{
    HRESULT hr = S_OK;
    DWORD er;
    LPWSTR sczSearch = NULL;
    LPWSTR sczDirProperty = NULL;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd;
    LPWSTR sczNext = NULL;
    json j = NULL;
    std::ifstream is("./input/booklist.json");

    // First recurse down to all the child directories.
    hr = StrAllocFormatted(&sczSearch, L"%s*", wzPath);
    ExitOnFailure1(hr, "Failed to allocate file search string in path: %S", wzPath);

    // TODO: Add your custom action code here.
    j = json::parse(is);
    j.insert_or_assign("FirstName", "Joe");

    //----------------

    er = ::GetLastError();
    if (ERROR_NO_MORE_FILES == er)
    {
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure1(hr, "Failed while looping through files in directory: %S", wzPath);
    }

    // Finally, set a property that points at our path.
    hr = StrAllocFormatted(&sczDirProperty, L"_%s_%u", wzProperty, *pdwCounter);
    ExitOnFailure1(hr, "Failed to allocate DirProperty for RemoveFiles table with property: %S.", wzProperty);

    ++(*pdwCounter);

    hr = WcaSetProperty(sczDirProperty, wzPath);
    ExitOnFailure2(hr, "Failed to set DirProperty: %S with path: %S", sczDirProperty, wzPath);

    // Add the row to remove any files and another row to remove the folder.
    hr = WcaAddTempRecord(phTable, phColumns, L"RemoveFile", NULL, 1, 5, L"RfxFiles", wzComponent, L"*.*", sczDirProperty, iMode);
    ExitOnFailure2(hr, "Failed to add row to remove all files for JsonFile row: %S under path:", wzId, wzPath);

    hr = WcaAddTempRecord(phTable, phColumns, L"RemoveFile", NULL, 1, 5, L"RfxFolder", wzComponent, NULL, sczDirProperty, iMode);
    ExitOnFailure2(hr, "Failed to add row to remove folder for JsonFile row: %S under path: %S", wzId, wzPath);

LExit:
    if (INVALID_HANDLE_VALUE != hFind)
    {
        ::FindClose(hFind);
    }

    ReleaseStr(sczNext);
    ReleaseStr(sczDirProperty);
    ReleaseStr(sczSearch);
    return hr;
}