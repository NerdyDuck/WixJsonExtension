
#include "stdafx.h"

using namespace jsoncons;

LPCWSTR vcsJsonFileQuery = L"SELECT `JsonFile`.`JsonFile`, `JsonFile`.`File`, `JsonFile`.`ElementPath`, `JsonFile`.`VerifyPath`, "
L"`JsonFile`.`Value`, `JsonFile`.`ValueType`, `JsonFile`.`Flags`, `JsonFile`.`Component_` FROM `JsonFile`,`Component`"
L"WHERE `JsonFile`.`Component_`=`Component`.`Component` ORDER BY `File`, `Sequence`";
enum eJsonFileQuery { jfqId = 1, jfqFile, jfqElementPath, jfqVerifyPath, jfqValue, jfqValueType, jfqFlags, jfqComponent };

static HRESULT UpdateJsonFile(
    __in_z LPCWSTR wzId,
    __in_z LPCWSTR wzFile,
    __in_z LPCWSTR wzElementPath,
    __in_z LPCWSTR wzVerifyPath,
    __in_z LPCWSTR wzValue,
    __in int iValueType,
    __in int iFlags,
    __in_z LPCWSTR wzComponent
);
void SetJsonPathValue(__in_z LPCWSTR wzFile, std::string sElementPath, __in_z LPCWSTR wzValue);

const int FLAG_DELETEVALUE = 0;
const int FLAG_SETVALUE = 1;
const int FLAG_ADDARRAYVALUE = 2;
const int FLAG_UNINSTALL = 3;
const int FLAG_PRESERVEDATE = 4;
const int FLAG_JSONPOINTER = 5;


extern "C" UINT WINAPI JsonFile(
    __in MSIHANDLE hInstall
)
{
    HRESULT hr = S_OK;
    hr = WcaInitialize(hInstall, "JsonFile");

    WcaLog(LOGMSG_STANDARD, "Entered JsonFile CA");
    //AssertSz(FALSE, "debug JsonFile");
        
    WcaLog(LOGMSG_STANDARD, "Created PMSIHANDLE hView");
    PMSIHANDLE hView;

    WcaLog(LOGMSG_STANDARD, "Created PMSIHANDLE hRec");
    PMSIHANDLE hRec;

    WcaLog(LOGMSG_STANDARD, "MSIHANDLE's created for JsonFile CA");

    LPWSTR sczId = NULL;
    LPWSTR sczFile = NULL;
    LPWSTR sczElementPath = NULL;
    LPWSTR sczVerifyPath = NULL;
    LPWSTR sczValue = NULL;
    LPWSTR sczComponent = NULL;

    int iValueType = 0;
    int iFlags = 0;
        
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
        WcaLog(LOGMSG_STANDARD, "Getting JsonFile Id.");
        hr = WcaGetRecordString(hRec, jfqId, &sczId);
        ExitOnFailure(hr, "Failed to get JsonFile identity.");

        WcaLog(LOGMSG_STANDARD, "Getting JsonFile File for Id:%ls", sczId);
        hr = WcaGetRecordFormattedString(hRec, jfqFile, &sczFile);
        ExitOnFailure1(hr, "failed to get File for JsonFile with Id: %s", sczId);

        WcaLog(LOGMSG_STANDARD, "Getting JsonFile ElementPath for Id:%ls", sczId);
        hr = WcaGetRecordString(hRec, jfqElementPath, &sczElementPath);
        ExitOnFailure1(hr, "Failed to get ElementPath for JsonFile with Id: %s", sczId);

        WcaLog(LOGMSG_STANDARD, "Getting JsonFile VerifyPath for Id:%ls", sczId);
        hr = WcaGetRecordString(hRec, jfqVerifyPath, &sczVerifyPath);
        ExitOnFailure1(hr, "Failed to get VerifyPath for JsonFile with Id: %s", sczId);

        WcaLog(LOGMSG_STANDARD, "Getting JsonFile Value for Id:%ls", sczId);
        hr = WcaGetRecordString(hRec, jfqValue, &sczValue);
        ExitOnFailure1(hr, "Failed to get Value for JsonFile with Id: %s", sczId);

        WcaLog(LOGMSG_STANDARD, "Getting JsonFile ValueType for Id:%ls", sczId);
        hr = WcaGetRecordInteger(hRec, jfqValueType, &iValueType);
        ExitOnFailure1(hr, "Failed to get ValueType for JsonFile with Id: %s", sczId);

        WcaLog(LOGMSG_STANDARD, "Getting JsonFile Flags for Id:%ls", sczId);
        hr = WcaGetRecordInteger(hRec, jfqFlags, &iFlags);
        ExitOnFailure1(hr, "Failed to get Flags for JsonFile with Id: %s", sczId);

        WcaLog(LOGMSG_STANDARD, "Getting JsonFile Component for Id:%ls", sczId);
        hr = WcaGetRecordString(hRec, jfqComponent, &sczComponent);
        ExitOnFailure(hr, "Failed to get remove folder component.");

        WcaLog(LOGMSG_STANDARD, "Updating JsonFile for Id:%ls", sczId);
        hr = UpdateJsonFile(sczId, sczFile, sczElementPath, sczVerifyPath, sczValue, iValueType, iFlags, sczComponent);
        ExitOnFailure2(hr, "Failed while navigating path: %S for row: %S", sczFile, sczId);
    }

    // reaching the end of the list is actually a good thing, not an error
    if (E_NOMOREITEMS == hr)
    {
        hr = S_OK;
    }
    ExitOnFailure(hr, "Failure occured while processing JsonFile table");

LExit:
    ReleaseStr(sczId);
    ReleaseStr(sczFile);
    ReleaseStr(sczElementPath);
    ReleaseStr(sczVerifyPath);
    ReleaseStr(sczValue);
    ReleaseStr(sczComponent);

    if (hView)
    {
        ::MsiCloseHandle(hView);
        WcaLog(LOGMSG_STANDARD, "Closed PMSIHANDLE hView");
    }

    if (hRec)
    {
        ::MsiCloseHandle(hRec);
        WcaLog(LOGMSG_STANDARD, "Closed PMSIHANDLE hRec");
    }

    DWORD er = SUCCEEDED(hr) ? ERROR_SUCCESS : ERROR_INSTALL_FAILURE;
    return WcaFinalize(er);
}


static HRESULT UpdateJsonFile(
    __in_z LPCWSTR wzId,
    __in_z LPCWSTR wzFile,
    __in_z LPCWSTR wzElementPath,
    __in_z LPCWSTR wzVerifyPath,
    __in_z LPCWSTR wzValue,
    __in int iValueType,
    __in int iFlags,
    __in_z LPCWSTR wzComponent
)
{
    HRESULT hr = S_OK;
    // DWORD er;
    ojson j = NULL;
    ::SetLastError(0);

    _bstr_t bFile(wzFile);
    char* cFile = bFile;

    _bstr_t bValue(wzValue);
    char* cValue = bValue;

    std::ifstream is(cFile);
    WcaLog(LOGMSG_STANDARD, "Created wifstream, %s", cFile);

    json_reader reader(is);

    std::error_code ec;
    reader.read(ec);

    if (ec)
    {
        WcaLog(LOGMSG_STANDARD, "%s on line %i and column %i", ec.message().c_str(), reader.line(), reader.column());
        std::cout << ec.message()
            << " on line " << reader.line()
            << " and column " << reader.column()
            << std::endl;
    }

    is.close();

    std::bitset<32> flags(iFlags);
    WcaLog(LOGMSG_STANDARD, "Using the following flags, %i, %s", iFlags, flags.test(FLAG_SETVALUE) ? "true" : "false");

    _bstr_t bElementPath(wzElementPath);
    char* cElementPath = bElementPath;

    std::string elementPath(cElementPath);

    WcaLog(LOGMSG_STANDARD, "Found ElementPath as %s", elementPath.c_str());
    elementPath = std::regex_replace(elementPath, std::regex("[\\[]"), "[");
    elementPath = std::regex_replace(elementPath, std::regex("[\\]]"), "]");
    WcaLog(LOGMSG_STANDARD, "Updated ElementPath to %s", elementPath.c_str());

    if (flags.test(FLAG_SETVALUE)) {
        SetJsonPathValue(wzFile, elementPath.c_str(), wzValue);
    }
    else if (flags.test(FLAG_DELETEVALUE)){

    }
    else if (flags.test(FLAG_JSONPOINTER)) {

    }

    return hr;
}

using namespace jsoncons::jsonpath;

void SetJsonPathValue(__in_z LPCWSTR wzFile, std::string sElementPath, __in_z LPCWSTR wzValue) {

    try
    {
        ojson j;

        _bstr_t bFile(wzFile);
        char* cFile = bFile;

        _bstr_t bValue(wzValue);
        char* cValue = bValue;

        std::ifstream is(cFile);

        is >> j;
        WcaLog(LOGMSG_STANDARD, "About to replace value, %s, %s", sElementPath.c_str(), cValue);

        json_replace(j, sElementPath, cValue);
        WcaLog(LOGMSG_STANDARD, "updated the json %s with values %s ", sElementPath.c_str(), cValue);

        std::ofstream os(wzFile,
            std::ios_base::out | std::ios_base::trunc);
        WcaLog(LOGMSG_STANDARD, "created output stream");

        pretty_print(j).dump(os);
        WcaLog(LOGMSG_STANDARD, "dumped output stream");

        os.close();
        WcaLog(LOGMSG_STANDARD, "closed output stream");
    }
    catch (std::exception& e)
    {
        WcaLog(LOGMSG_STANDARD, "encountered error %s", e.what());
        throw;
    }
}
