#include <iostream>
#include <fstream>
#include <ctime>
#include <string>
#include <sstream>
#include <sys/stat.h>
#include <stdlib.h>

#include "clientsocket.h"
#include "socketexception.h"

#include "jfyconnection.h"

using namespace std;

static const string JFY_CONNECTION_PORT = "/dev/ttyUSB0";
static const string LOGSTASH_HOSTNAME = "localhost";
static const int LOGSTASH_PORT = 7022;

static const string CACHE_FILENAME = "log_cache.txt";

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
	Jfy::Connection conn(JFY_CONNECTION_PORT);

	if ( !conn.init() ) {
		cerr << "Cannot initialise the connection." << endl;
		return 1;
	}

	// Get the data
	Jfy::InverterData data;

	if(!conn.readNormalInfo(&data)) {
		cerr << "Data not correct. Not sending to logstash.";
		return 1;
	}

	// Create log string
	string log = buildLog(&data);

	try
	{
		try
		{
			// Write to logstash
			ClientSocket client_socket(LOGSTASH_HOSTNAME, LOGSTASH_PORT);

			client_socket << log;

			// std::cout << log << "Sent\n";

			// Log has been sent; attempt to flush through contents of cache file

			string line;

			ifstream cacheFile(CACHE_FILENAME.c_str());

			if (cacheFile.is_open())
			{
				int linesSent = 0;

				while (getline(cacheFile,line))
			    	{
			      		client_socket << line << "\n";
					
					linesSent++;
			    	}

				cout << "Total lines sent from cache: " << linesSent << "\n";

			    cacheFile.close();
			}

			// Logs have been sent; remove the cache file
			std::remove(CACHE_FILENAME.c_str());


		}
		catch (SocketException& e)
		{
      			std::cout << "Error writing to logging server:\n\"" << e.description() << "\". Writing log cache file.\n";

			// write to cache
			ofstream cacheFile;
			cacheFile.open(CACHE_FILENAME.c_str(), ios::out | ios::app);
			cacheFile << log;
			cacheFile.close();
		}

	}
	catch (SocketException& e)
	{
		std::cout << "Error connecting to logging server:\n\"" << e.description() << "\"\n";
	}

	conn.close();

	return 0;
}

