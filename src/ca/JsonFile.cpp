
#include "stdafx.h"

using namespace jsoncons;

LPCWSTR vcsJsonFileQuery = L"SELECT `JsonFile`.`JsonFile`, `JsonFile`.`File`, `JsonFile`.`ElementPath`, `JsonFile`.`VerifyPath`, "
L"`JsonFile`.`Name`, `JsonFile`.`Value`, `JsonFile`.`ValueType`, `JsonFile`.`Flags`, `JsonFile`.`Component_` FROM `JsonFile`,`Component`"
L"WHERE `JsonFile`.`Component_`=`Component`.`Component` ORDER BY `File`, `Sequence`";
enum eJsonFileQuery { jfqId = 1, jfqFile, jfqElementPath, jfqVerifyPath, jfqName, jfqValue, jfqValueType, jfqFlags, jfqComponent };

static HRESULT UpdateJsonFile(
    __in_z LPCWSTR wzId,
    __in_z LPCWSTR wzFile,
    __in_z LPCWSTR wzElementPath,
    __in_z LPCWSTR wzVerifyPath,
    __in_z LPCWSTR wzName,
    __in_z LPCWSTR wzValue,
    __in int iValueType,
    __in int iFlags,
    __in_z LPCWSTR wzComponent
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
    LPWSTR sczFile = NULL;
    LPWSTR sczElementPath = NULL;
    LPWSTR sczVerifyPath = NULL;
    LPWSTR sczName = NULL;
    LPWSTR sczValue = NULL;
    LPWSTR sczComponent = NULL;
    LPWSTR sczProperty = NULL;
    int iValueType = 0;
    int iFlags = 0;

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
        ExitOnFailure(hr, "Failed to get JsonFile identity.");

        hr = WcaGetRecordFormattedString(hRec, jfqFile, &sczFile);
        ExitOnFailure1(hr, "failed to get File for JsonFile with Id: %s", sczId);

        hr = WcaGetRecordString(hRec, jfqElementPath, &sczElementPath);
        ExitOnFailure1(hr, "Failed to get ElementPath for JsonFile with Id: %s", sczId);

        hr = WcaGetRecordString(hRec, jfqVerifyPath, &sczVerifyPath);
        ExitOnFailure1(hr, "Failed to get VerifyPath for JsonFile with Id: %s", sczId);

        hr = WcaGetRecordString(hRec, jfqName, &sczName);
        ExitOnFailure1(hr, "Failed to get Name for JsonFile with Id: %s", sczId);

        hr = WcaGetRecordString(hRec, jfqValue, &sczValue);
        ExitOnFailure1(hr, "Failed to get Value for JsonFile with Id: %s", sczId);

        hr = WcaGetRecordInteger(hRec, jfqValueType, &iValueType);
        ExitOnFailure1(hr, "Failed to get ValueType for JsonFile with Id: %s", sczId);

        hr = WcaGetRecordInteger(hRec, jfqFlags, &iFlags);
        ExitOnFailure1(hr, "Failed to get Flags for JsonFile with Id: %s", sczId);

        hr = WcaGetRecordString(hRec, jfqComponent, &sczComponent);
        ExitOnFailure(hr, "Failed to get remove folder component.");

        hr = UpdateJsonFile(sczId, sczFile, sczElementPath, sczVerifyPath, sczName, sczValue, iValueType, iFlags, sczComponent);
        ExitOnFailure2(hr, "Failed while navigating path: %S for row: %S", sczFile, sczId);
    }

    // reaching the end of the list is actually a good thing, not an error
    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure occured while processing JsonFile table");

LExit:
    ReleaseStr(sczProperty);
    ReleaseStr(sczComponent);
    ReleaseStr(sczId);

    DWORD er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


static HRESULT UpdateJsonFile(
    __in_z LPCWSTR wzId,
    __in_z LPCWSTR wzFile,
    __in_z LPCWSTR wzElementPath,
    __in_z LPCWSTR wzVerifyPath,
    __in_z LPCWSTR wzName,
    __in_z LPCWSTR wzValue,
    __in int iValueType,
    __in int iFlags,
    __in_z LPCWSTR wzComponent
)
{
    HRESULT hr = S_OK;
    DWORD er;
    LPWSTR sczSearch = NULL;
    LPWSTR sczDirProperty = NULL;
    HANDLE hFind = INVALID_HANDLE_VALUE;
    WIN32_FIND_DATAW wfd;
    LPWSTR sczNext = NULL;
    wjson j = NULL;
    std::wifstream is(wzFile);

    // TODO: Add your custom action code here.
    j = wjson::parse(is);
    j.insert_or_assign(wzName, wzValue);

    std::wofstream os(wzFile,
        std::ios_base::out | std::ios_base::trunc);

    j.dump(os);

    os.close();

    er = ::GetLastError();
    if (ERROR_NO_MORE_FILES == er)
    {
        hr = S_OK;
    }
    else
    {
        hr = HRESULT_FROM_WIN32(er);
        ExitOnFailure1(hr, "Failed while updating .json file: %S", wzFile);
    }

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
