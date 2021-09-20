////////////////////////////////////////////////////////////////////////////////////////////////
//                                                                                            //
//   Program for METAR and TAF database receiving from Aviation Weather Center                //
//   https://aviationweather.gov/adds/dataserver_current/current/ for departure airport ICAO, //
//   arriving airport ICAO and radius zone near flight path with follow TAFS and METAR data   //
//   .csv files parsing                                                                       //
//                                                             Barracuda_marina, 25.05.2021   //
//                                                                           ver. 0.999a beta //
////////////////////////////////////////////////////////////////////////////////////////////////

#define CURL_STATICLIB

#include "METAF.hpp"
#include <iostream>
#include <cstring>
#include <sstream>
#include "curl\curl.h"

#ifdef _DEBUG

#pragma comment (lib, "curl/libcurl_a_debug.lib")
#else 
#pragma comment (lib, "curl/libcurl_a.lib")
#endif // 

#pragma comment (lib, "Normaliz.lib")
#pragma comment (lib, "Ws2_32.lib")
#pragma comment (lib, "Wldap32.lib")
#pragma comment (lib, "Crypt32.lib")
#pragma comment (lib, "advapi32.lib")

using namespace std;
using namespace metaf;

// --------------------------------------------------------------------------------------------

char ap_departure[5] = "UKOO";
char ap_arriving[5] = "UKBB";
char search_radius[3] = "50";
char hours_before_now[3] = "2";

//---------------------------------------------------------------------------------------------

size_t write_data(char* ptr, size_t size, size_t nmemb, FILE* data)
{
    return fwrite(ptr, size, nmemb, data);
}
//---------------------------------------------------------------------------------------------

// Test function for flightpath data input:

int test_input(void);

//---------------------------------------------------------------------------------------------

string errorMessage(ReportError error) {
    switch (error) {
    case ReportError::NONE:
        return "no error, parsed succesfully";

    case ReportError::EMPTY_REPORT:
        return "empty report";

    case ReportError::EXPECTED_REPORT_TYPE_OR_LOCATION:
        return "expected report type or ICAO location";

    case ReportError::EXPECTED_LOCATION:
        return "expected ICAO location";

    case ReportError::EXPECTED_REPORT_TIME:
        return "expected report time";

    case ReportError::EXPECTED_TIME_SPAN:
        return "expected time span";

    case ReportError::UNEXPECTED_REPORT_END:
        return "unexpected report end";

    case ReportError::UNEXPECTED_GROUP_AFTER_NIL:
        return "unexpected group after NIL";

    case ReportError::UNEXPECTED_GROUP_AFTER_CNL:
        return "unexpected group after CNL";

    case ReportError::UNEXPECTED_NIL_OR_CNL_IN_REPORT_BODY:
        return "unexpected NIL or CNL in report body";

    case ReportError::AMD_ALLOWED_IN_TAF_ONLY:
        return "AMD is allowed in TAF only";

    case ReportError::CNL_ALLOWED_IN_TAF_ONLY:
        return "CNL is allowed in TAF only";

    case ReportError::MAINTENANCE_INDICATOR_ALLOWED_IN_METAR_ONLY:
        return "Maintenance indicator is allowed only in METAR reports";

    case ReportError::REPORT_TOO_LARGE:
        return "Report has too many groups and may be malformed";
    }
}


string reportTypeMessage(ReportType reportType) {
    switch (reportType) {
    case ReportType::UNKNOWN:
        return "unable to detect";

    case ReportType::METAR:
        return "METAR";

    case ReportType::TAF:
        return "TAF";
    }
}

class MyVisitor : public Visitor<string> {
    virtual string visitKeywordGroup(
        const KeywordGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Keyword: " + rawString);
    }

    virtual string visitLocationGroup(
        const LocationGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("ICAO location: " + rawString);
    }

    virtual string visitReportTimeGroup(
        const ReportTimeGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Report Release Time: " + rawString);
    }

    virtual string visitTrendGroup(
        const TrendGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Trend Header: " + rawString);
    }

    virtual string visitWindGroup(
        const WindGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Wind: " + rawString);
    }

    virtual string visitVisibilityGroup(
        const VisibilityGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Visibility: " + rawString);
    }

    virtual string visitCloudGroup(
        const CloudGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Cloud Data: " + rawString);
    }

    virtual string visitWeatherGroup(
        const WeatherGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Weather Phenomena: " + rawString);
    }

    virtual string visitTemperatureGroup(
        const TemperatureGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Temperature and Dew Point: " + rawString);
    }

    virtual string visitPressureGroup(
        const PressureGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Pressure: " + rawString);
    }

    virtual string visitRunwayStateGroup(
        const RunwayStateGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("State of Runway:" + rawString);
    }

    virtual string visitSeaSurfaceGroup(
        const SeaSurfaceGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Sea Surface: " + rawString);
    }

    virtual string visitMinMaxTemperatureGroup(
        const MinMaxTemperatureGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Minimum/Maximum Temperature: " + rawString);
    }

    virtual string visitPrecipitationGroup(
        const PrecipitationGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Precipitation: " + rawString);
    }

    virtual string visitLayerForecastGroup(
        const LayerForecastGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Atmospheric Layer Forecast: " + rawString);
    }

    virtual string visitPressureTendencyGroup(
        const PressureTendencyGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Pressure Tendency: " + rawString);
    }

    virtual string visitCloudTypesGroup(
        const CloudTypesGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Cloud Types: " + rawString);
    }

    virtual string visitLowMidHighCloudGroup(
        const LowMidHighCloudGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Low, middle, and high cloud layers: " + rawString);
    }

    virtual string visitLightningGroup(
        const LightningGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Lightning data: " + rawString);
    }

    virtual string visitVicinityGroup(
        const VicinityGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Events in vicinity: " + rawString);
    }

    virtual string visitMiscGroup(
        const MiscGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Miscellaneous Data: " + rawString);
    }

    virtual string visitUnknownGroup(
        const UnknownGroup& group,
        ReportPart reportPart,
        const string& rawString)
    {
        (void)group; (void)reportPart;
        return ("Not recognised by the parser: " + rawString);
    }
};

// ===================================================================================================
// |                               MAIN function start                                               |
// ===================================================================================================

int main(void)
{

    // Define and open local files for METAR and TAF ------------------------------------------------

    const string header_filename_metars = "files/metars.txt";
    const string body_filename_metars = "files/metars.csv";
    const string header_filename_tafs = "files/tafs.txt";
    const string body_filename_tafs = "files/tafs.csv";
    const string filename_metafs = "files/metafs.txt";

    FILE* header_file_metars = fopen(header_filename_metars.c_str(), "w");
    if (header_file_metars == NULL)
        return -1;

    FILE* body_file_metars = fopen(body_filename_metars.c_str(), "w");
    if (body_file_metars == NULL)
        return -1;

    FILE* header_file_tafs = fopen(header_filename_tafs.c_str(), "w");
    if (header_file_tafs == NULL)
        return -1;

    FILE* body_file_tafs = fopen(body_filename_tafs.c_str(), "w");
    if (body_file_tafs == NULL)
        return -1;

    // Query data input ---------------------------------------------------------------------------

    system("cls");
    std::cout << "\nWelcome Sir." << std::endl;
    std::cout << "\nPlease input flight path data or press <Ctrl+C> to exit: " << std::endl;
    std::cout << "\nDeparture AP ICAO: ";
    std::cin >> ap_departure;
    std::cout << "Arriving AP ICAO: ";
    std::cin >> ap_arriving;
    std::cout << "Radius search (nm): ";
    std::cin >> search_radius;
    std::cout << "Hours before now: ";
    std::cin >> hours_before_now;

    // Indicator bar ------------------------------------------------------------------------------

    std::cout << "\nReceiving  data  ";
    for (int ccc = 0; ccc < 75; ccc++) {
        for (int cccc = 0; cccc < 7000000; cccc++);
        std::cout << "|";
    }
    std::cout << std::endl;


    // Module to receive data files from AWC =============================================================================

    CURL* curl_handle = curl_easy_init();
    if (curl_handle)
    {
        // METARS part -------------------------------------------------------------------------------------------------------------

        char url_metars_01[] = "https://aviationweather.gov/adds/dataserver_current/httpparam?dataSource=metars&requestType=retrieve&format=csv&flightPath=";
        char url_metars_02[] = "&hoursBeforeNow=";

        stringstream url_metars;
        url_metars << url_metars_01 << search_radius << ";" << ap_departure << ";" << ap_arriving << url_metars_02 << hours_before_now;
        string str_url_metars = url_metars.str();

        // debug block
        // test_input();
        //cout << str_url_metars << endl; //debug url

        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
        curl_easy_setopt(curl_handle, CURLOPT_URL, str_url_metars.c_str());

        // save METARS file.csv
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, body_file_metars);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

        // save header METARS file.txt
        curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, header_file_metars);

        CURLcode res_metars = curl_easy_perform(curl_handle);
        if (res_metars != CURLE_OK)
            cout << "curl_easy_perform() failed: %s\n" << curl_easy_strerror(res_metars) << endl;

        // TAFS part --------------------------------------------------------------------------------------------------------------

        char url_tafs_01[] = "https://aviationweather.gov/adds/dataserver_current/httpparam?dataSource=tafs&requestType=retrieve&format=csv&flightPath=";
        char url_tafs_02[] = "&hoursBeforeNow=";

        stringstream url_tafs;
        url_tafs << url_tafs_01 << search_radius << ";" << ap_departure << ";" << ap_arriving << url_tafs_02 << hours_before_now;
        string str_url_tafs = url_tafs.str();

        // debug block
        // test_input();
        // cout << str_url_tafs << endl; //debug url

        curl_easy_setopt(curl_handle, CURLOPT_SSL_VERIFYPEER, FALSE);
        curl_easy_setopt(curl_handle, CURLOPT_URL, str_url_tafs.c_str());

        // save TAFS file.csv
        curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, body_file_tafs);
        curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, write_data);

        // save header TAFS file.txt
        curl_easy_setopt(curl_handle, CURLOPT_WRITEHEADER, header_file_tafs);

        CURLcode res_tafs = curl_easy_perform(curl_handle);
        if (res_tafs != CURLE_OK)
            cout << "curl_easy_perform() failed: %s\n" << curl_easy_strerror(res_tafs) << endl;

        curl_easy_cleanup(curl_handle);
    }

    fclose(header_file_metars);
    fclose(body_file_metars);
    fclose(header_file_tafs);
    fclose(body_file_tafs);

    cout << "Done!" << "\nFlightpath weather data were stored in subfolder </files> in the files: " << body_filename_metars << ", " << body_filename_tafs << endl;
    cout << endl;
    system("pause");
    cout << "\nSo ... \n";


 // !!!!!!!!!!!!!!!!!!!!!!!!!!!!!  DATA extracting from file receiving  !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
 
 // === METAR section =====================================================================================

    int i = 0;
    char str_csv_metar1[50000];
    char str_txt_metar1[300];

    FILE* fp_csv_m1 = fopen("files/metars.csv", "r");   // Path can to be changed for release ver.
    FILE* fp_txt_m1 = fopen("files/metaf.txt", "w");    // Path can to be changed for release ver.
    if (fp_csv_m1 != NULL) {
        while ((str_csv_metar1[i] = getc(fp_csv_m1)) != EOF) i++;
        str_csv_metar1[i] = '\0';
    }

    //   cout << str_csv_metar1; // debug

    //  ----- str_m - METARS string from file request -----

    string str_m = string(str_csv_metar1);
    int a;

    // METAR for departure airport ------------------------------------------------------------------------

    stringstream ap_departure_metar_tmp;
    ap_departure_metar_tmp << "," << ap_departure << ",";
    string ap_departure_metar_tmpl = ap_departure_metar_tmp.str();
    string ap_departure_metar_orig = ap_departure;

    i = str_m.find(ap_departure_metar_tmpl);

    //   cout << i; // debug

    string beginStr_m_dep = str_m.substr(0, i);

    //   cout << beginStr_m_dep << endl; // debug

    i = beginStr_m_dep.find(ap_departure_metar_orig);
    string endStr_m_dep = str_m.substr(i, beginStr_m_dep.length() - i);
    cout << "METAR " << endStr_m_dep << endl;
    if (endStr_m_dep.empty())
    {
        cout << "app" << endl;
    }

    // cout << str_txt_metar1 << endl; // debug

    fputs(endStr_m_dep.c_str(), fp_txt_m1);
    fputs("\n", fp_txt_m1);

    // METAR for arriving airport ------------------------------------------------------------------------

    stringstream ap_arriving_metar_tmp;
    ap_arriving_metar_tmp << "," << ap_arriving << ",";
    string ap_arriving_metar_tmpl = ap_arriving_metar_tmp.str();
    string ap_arriving_metar_orig = ap_arriving;

    i = str_m.find(ap_arriving_metar_tmpl);

    //   cout << i; // debug

    string beginStr_m_arr = str_m.substr(0, i);

    //   cout << beginStr_m_arr << endl; // debug

    i = beginStr_m_arr.find(ap_arriving_metar_orig);
    string endStr_m_arr = str_m.substr(i, beginStr_m_arr.length() - i);
    cout << "METAR " << endStr_m_arr << endl;
    if (endStr_m_arr.empty())
    {
        cout << "app" << endl;
    }

    // cout << str_txt_metar1 << endl; // debug

    fputs(endStr_m_arr.c_str(), fp_txt_m1);
    fputs("\n", fp_txt_m1);


//== TAF section ========================================================================================

    i = 0;
    char str_csv_taf1[300000];
    
    FILE* fp_csv_t1 = fopen("files/tafs.csv", "r");  // Path can to be changed for release ver.

    if (fp_csv_t1 != NULL) {
        while ((str_csv_taf1[i] = getc(fp_csv_t1)) != EOF) i++;
        str_csv_taf1[i] = '\0';
    }

    //   cout << str_csv_taf1; // debug

    // ----- str_t - TAFS string from file after request  ------

    string str_t = string(str_csv_taf1);

    // cout << str_t; // debug
    // system("pause"); // debug

    
    // TAF for departure airport ------------------------------------------------------------------------

    stringstream ap_departure_taf_tmp;
    ap_departure_taf_tmp << "," << ap_departure << ",";
    string ap_departure_taf_tmpl = ap_departure_taf_tmp.str();
    string ap_departure_taf_orig = ap_departure;

    /*  debug block
        cout << ap_departure << " " << ap_departure_taf_tmpl << " " << ap_departure_taf_orig << endl;
        system("pause");
        // end debug block
    */

    i = str_t.find(ap_departure_taf_tmpl);

    //   cout << i; // debug

    string beginStr_t_dep = str_t.substr(0, i);

    //   cout << beginStr_t_dep << endl; // debug

    i = beginStr_t_dep.find(ap_departure_taf_orig);

    //   cout << i; // debug

    string endStr_t_dep = str_t.substr(i, beginStr_t_dep.length() - i);
    cout << "TAF " << endStr_t_dep << endl;

    fputs("TAF ", fp_txt_m1);
    fputs(endStr_t_dep.c_str(), fp_txt_m1);
    fputs("\n", fp_txt_m1);

    // TAF for arriving airport ------------------------------------------------------------------------

    stringstream ap_arriving_taf_tmp;
    ap_arriving_taf_tmp << "," << ap_arriving << ",";
    string ap_arriving_taf_tmpl = ap_arriving_taf_tmp.str();
    string ap_arriving_taf_orig = ap_arriving;

    /*  debug block
        cout << ap_arriving << " " << ap_arriving_taf_tmpl << " " << ap_arriving_taf_orig << endl;
        system("pause");
        // end debug block
    */

    i = str_t.find(ap_arriving_taf_tmpl);

    // cout << i; //debug
    // system("pause");  //debug

    string beginStr_t_arr = str_t.substr(0, i);

    // cout << beginStr_t_arr << endl;  //debug 

    i = beginStr_t_arr.find(ap_arriving_taf_orig); 

    // cout << i;  //debug
    // system("pause");  //debug

    string endStr_t_arr = str_t.substr(i, beginStr_t_arr.length() - i);
    cout << "TAF " << endStr_t_arr << endl;
    
    // system("pause");  //debug

    fputs("TAF ", fp_txt_m1);
    fputs(endStr_t_arr.c_str(), fp_txt_m1);
    fputs("\n", fp_txt_m1);

    system("pause"); //debug

 //== Parsing section =====================================================================================

    // Departure airport METAR report --------------------------------------------------------------------

    cout << "\nParsing report: " << endStr_m_dep << endl;
    const auto result1 = Parser::parse(endStr_m_dep.c_str());
    cout << "Parse error: ";
    cout << errorMessage(result1.reportMetadata.error) << "\n";
    cout << "Detected report type: ";
    cout << reportTypeMessage(result1.reportMetadata.type) << "\n";
    cout << result1.groups.size() << " groups parsed\n";
    MyVisitor visitor1;
    for (const auto groupInfo : result1.groups) {
        cout << visitor1.visit(groupInfo) << "\n";
    }
    system("pause");

    // Departure airport TAF report --------------------------------------------------------------------

    cout << "\nParsing report: " << endStr_t_dep.c_str() << "\n";
    const auto result2 = Parser::parse(endStr_t_dep.c_str());
    cout << "Parse error: ";
    cout << errorMessage(result2.reportMetadata.error) << "\n";
    cout << "Detected report type: ";
    cout << reportTypeMessage(result2.reportMetadata.type) << "\n";
    cout << result2.groups.size() << " groups parsed\n";
    MyVisitor visitor2;
    for (const auto groupInfo : result2.groups) {
        cout << visitor2.visit(groupInfo) << "\n";
    }
    system("pause");

    // Arriving airport METAR report --------------------------------------------------------------------

    cout << "\nParsing report: " << endStr_m_arr.c_str() << "\n";
    const auto result3 = Parser::parse(endStr_m_arr.c_str());
    cout << "Parse error: ";
    cout << errorMessage(result3.reportMetadata.error) << "\n";
    cout << "Detected report type: ";
    cout << reportTypeMessage(result3.reportMetadata.type) << "\n";
    cout << result3.groups.size() << " groups parsed\n";
    MyVisitor visitor3;
    for (const auto groupInfo : result3.groups) {
        cout << visitor3.visit(groupInfo) << "\n";
    }
    system("pause");

    // Arriving airport TAF report --------------------------------------------------------------------

    cout << "\nParsing report: " << endStr_t_arr.c_str() << "\n";
    const auto result4 = Parser::parse(endStr_t_arr.c_str());
    cout << "Parse error: ";
    cout << errorMessage(result4.reportMetadata.error) << "\n";
    cout << "Detected report type: ";
    cout << reportTypeMessage(result4.reportMetadata.type) << "\n";
    cout << result4.groups.size() << " groups parsed\n";
    MyVisitor visitor4;
    for (const auto groupInfo : result4.groups) {
        cout << visitor4.visit(groupInfo) << "\n";
    }
    system("pause");


    fclose(fp_txt_m1);
    fclose(fp_csv_m1);
    fclose(fp_csv_t1);

    cout << "\nMERARS and TAFS for departure and arriving airports were stored in the file: " << filename_metafs << endl;
    cout << "\nBye, Cap!\n\n";
    system("pause");
    return 0;
}



//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//                                                                                                       //
//                  E   N   D           O   F           P   R   O   G   R   A   M                        //
//                                                                                                       //
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!//
