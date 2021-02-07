#include "pch.h"
#include "PostDLLCpp.h"

#define DEBUG

std::string getCurrentExecutablePath();
std::string readPostURLFromFile(std::string path);

std::ifstream openFileStreamToInputPostFile(std::string path);
std::ofstream openFileStreamToOutputResponseFile(std::string path);

void __LOG_DBG(const char* format, ...);
void _runPostRequestAndSaveToFile();

std::size_t writeToOutputPostResponseFileStream(char* bodyContent, size_t size, size_t nmemb, void* ofstream);
std::size_t readInputPostFileStream(char* buffer, size_t size, size_t nitems, void* ifstream);

POSTDLLCPP_API void runPostRequestAndSaveToFile(REQUEST_ERR_INFO &errInfo)
{
    try
    {
        _runPostRequestAndSaveToFile();

        errInfo.failed = false;
    }

    catch (std::runtime_error err)
    {
        errInfo.cause = err.what();
        errInfo.failed = true;
    }
}

void _runPostRequestAndSaveToFile()
{
    CURL* curl = curl_easy_init();

    if (curl) 
    {
        std::string currentExePath = getCurrentExecutablePath();
        std::string postURL = readPostURLFromFile(currentExePath);

        __LOG_DBG("The current .exe directory is %s", currentExePath.c_str());
        __LOG_DBG("Loaded %s as the POST URL", postURL.c_str());

        std::ifstream inputPostFileStream = openFileStreamToInputPostFile(currentExePath);
        std::ofstream outputPostFileStream = openFileStreamToOutputResponseFile(currentExePath);

        __LOG_DBG("CURL context is starting");

        curl_easy_setopt(curl, CURLOPT_URL, postURL.c_str());
        curl_easy_setopt(curl, CURLOPT_POST, 1);
        curl_easy_setopt(curl, CURLOPT_WRITEFUNCTION, readInputPostFileStream);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, &inputPostFileStream);
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, writeToOutputPostResponseFileStream);
        curl_easy_setopt(curl, CURLOPT_READDATA, &outputPostFileStream);

#ifdef DEBUG
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1);
#endif

        __LOG_DBG("Request has been set up, starting");

        CURLcode response = curl_easy_perform(curl);

        curl_easy_cleanup(curl);

        __LOG_DBG("Request cleaned");

        if (response != CURLE_OK)
        {
            throw std::runtime_error(response + " - Failed to run POST request for URL " + postURL);
        }
    }
}

std::size_t writeToOutputPostResponseFileStream(char* bodyContent, size_t size, size_t nmemb, void* ofstream)
{
    std::ofstream* outputPostFileStream = (std::ofstream*)ofstream;
    outputPostFileStream->write(bodyContent, nmemb);

    __LOG_DBG("Writen %d bytes to output file", nmemb);

    return nmemb;
}

std::size_t readInputPostFileStream(char* buffer, size_t size, size_t nitems, void* ifstream)
{
    size_t totalSize = size * nitems;

    std::ifstream* inputPostFileStream = (std::ifstream*)ifstream;
    inputPostFileStream->read(buffer, totalSize);

    __LOG_DBG("%d bytes has been transfered from input POST request file to the buffer", totalSize);

    return totalSize;
}

std::string readPostURLFromFile(std::string path)
{
    std::ifstream postURLIFStream(path + "posturl.txt");
    
    if (postURLIFStream.is_open())
    {
        std::string lineRead;

        while (std::getline(postURLIFStream, lineRead))
        {
            if (lineRead.length() > 0)
            {
                postURLIFStream.close();

                return lineRead;
            }
        }
    }

    postURLIFStream.close();

    throw std::runtime_error("No POST URL found in posturl.txt or the file doesn't exist at the executable directory.");
}

std::ifstream openFileStreamToInputPostFile(std::string path)
{
    std::ifstream postInputIFStream(path + "postbody.bin");
    
    if (!postInputIFStream.is_open())
    {
        postInputIFStream.close();
        throw std::runtime_error("Input POST body file \"postbody.bin\" not present within the executable directory.");
    }

    return postInputIFStream;
}

std::ofstream openFileStreamToOutputResponseFile(std::string path)
{
    std::ofstream postOutputOFStream(path + "response.bin");

    if (!postOutputOFStream.is_open())
    {
        postOutputOFStream.close();
        throw std::runtime_error("Couldn't open output POST response file \"response.bin\".");
    }

    return postOutputOFStream;
}

std::string getCurrentExecutablePath()
{
    char executablePath[MAX_PATH];

    unsigned long writenPathLen = GetModuleFileNameA(nullptr, executablePath, MAX_PATH);

    std::string strFullExePath(executablePath);

    std::size_t posOfLastPathSeparator = strFullExePath.find_last_of("\\");

    if (posOfLastPathSeparator != std::string::npos)
    {
        //Substring from 0 to the last \ (also grabs the \)
        return strFullExePath.substr(0, posOfLastPathSeparator + 1);
    }

    return "";
}

void __LOG_DBG(const char* format, ...)
{
#ifdef DEBUG
    char debugMsg[] = "DEBUG - ";
    char newLine[] = "\n";

    size_t debugMsgLen = strlen(debugMsg);
    size_t newLineLen = strlen(newLine);
    size_t formatLen = strlen(format);
    size_t nullTermLen = 1;

    char* formatNL = new char[debugMsgLen + formatLen + newLineLen + nullTermLen];
    std::strcpy(formatNL, "");

    std::strcat(formatNL, debugMsg);
    std::strcat(formatNL, format);
    std::strcat(formatNL, newLine);

    va_list argptr;
    va_start(argptr, format);
    vfprintf(stdout, formatNL, argptr);
    va_end(argptr);

    delete[] formatNL;
#endif
}