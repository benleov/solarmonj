#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <stdlib.h>

#include "INIReader.h"

#include "clientsocket.h"
#include "socketexception.h"
#include "jfyconnection.h"

using namespace std;

static const string INI_CONFIG_PATH = "jfyconfig.ini";

static const string DEFAULT_JFY_DEVICE = "/dev/ttyUSB0";
static const string DEFAULT_LOGSTASH_HOSTNAME = "localhost";
static const int DEFAULT_LOGSTASH_PORT = 7022;

// File that will contain cached jfy data
static const string JFY_CACHE_FILENAME = "jfy_data_cache.txt";
static const string JFY_CACHE_TEMP_FILENAME = "jfy_data_cache.tmp";
/**
 * Creates a line of data containing all inverter information to be processed by logstash.
 */
static string buildLog(Jfy::InverterData* data)
{
	stringstream ss;

	// Get the current time
	time_t now = time(0);

	ss << now << "," <<   data->temperature << "," << data->energyCurrent << "," << data->energyToday << "," << data->pvoltageAc << "," << data->voltageDc << "," << data->voltageAc << "," << data->frequency << "," << data->current << "\n";

	return ss.str();

}

int main(int argc, char** argv)
{
    // Parse configuration from ini file

     INIReader reader(INI_CONFIG_PATH);

     if (reader.ParseError() < 0) {
        cerr << "Can't load " << INI_CONFIG_PATH << endl;
        return 1;
     }

    string jfyDevice = reader.Get("settings", "device", DEFAULT_JFY_DEVICE.c_str());
    string logstashHost = reader.Get("settings", "logstashHost", DEFAULT_LOGSTASH_HOSTNAME.c_str());
    int logstashPort = reader.GetInteger("settings", "logstashPort", DEFAULT_LOGSTASH_PORT);

    cout << "Using device: " << jfyDevice << endl;
    cout << "Sending to logstash: " << logstashHost << ":" << logstashPort << endl;

	Jfy::Connection conn(jfyDevice);

	if ( !conn.init() ) {
		cerr << "Cannot initialise the connection." << endl;
		return 1;
	}

	// Get the data
	Jfy::InverterData data;

    // Ignore invalid data
	if(!conn.readNormalInfo(&data)) {
		cerr << "Data not correct. Not sending to logstash.";
		return 1;
	}

	// Create log string
	string log = buildLog(&data);

    try
    {
        // Write to logstash
        ClientSocket client_socket(logstashHost, logstashPort);
        client_socket << log;

        // Log has been sent; attempt to flush through contents of cache file

        string line;

        ifstream cacheFile(JFY_CACHE_FILENAME.c_str());

        if (cacheFile.is_open())
        {
            int linesSent = 0;

            try
            {

                while (getline(cacheFile,line))
                {
                    client_socket << line << "\n";
                    linesSent++;
                }

                // All data has been sent; remove the cache file
                std::remove(JFY_CACHE_FILENAME.c_str());

                cout << "All lines sent from cache. Total lines: " << linesSent << endl;
            }
            catch (SocketException& e)
            {
                // A socket exception occurred while processing the cached data.
                // Remove the lines that were sent successfully by copying remaining lines to a temp file,
                // removing the old file, and renaming temp to original

                ofstream tempFile;
                tempFile.open(JFY_CACHE_TEMP_FILENAME.c_str(), ios::out); // open temp file
                tempFile << line << endl;   // write current line

                int linesMoved = 1;

                // write remaining lines
                while (getline(cacheFile,line))
                {
                    tempFile << line << "\n";
                    linesMoved++;
                }

                // close temp file
                tempFile.close();

                // remove old cache file
                std::remove(JFY_CACHE_FILENAME.c_str());

                // rename temp to cache file
                std::rename(JFY_CACHE_TEMP_FILENAME.c_str(), JFY_CACHE_FILENAME.c_str());
                cout << "Error occurred resending cache. Total sent:  " << linesSent <<  "Lines remaining: " << linesMoved << "\n";
            }



            cacheFile.close();
        }




    }
    catch (SocketException& e)
    {
        std::cout << "Error writing to logging server:\n\"" << e.description() << "\". Writing log cache file.\n";

        // Write to cache
        ofstream cacheFile;
        cacheFile.open(JFY_CACHE_FILENAME.c_str(), ios::out | ios::app);
        cacheFile << log;
        cacheFile.close();
    }

    // Close connection to jfy device
	conn.close();

	return 0;
}

