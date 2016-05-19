# solarmonj
Forked from https://github.com/jcroucher/solarmonj By Adam Gray & John Croucher.

This project has been extended to send data from an SUNTWINS 5000TL Series Inverter over USB converter  
to [Logstash](https://www.elastic.co/products/logstash), via a TCP/IP socket.

In addition to this, it can be configured via an ini file and will cache data if Logstash cannot be reached periodically.

### Example Logstash Dashboard

![](https://raw.githubusercontent.com/benleov/solarmonj/master/images/kibana_dashboard.png)

### Requirements

To run this project you will need:

* [SUNTWINS 5000TL Series Inverter] (http://jfytech.com.au/service/)
* [RS232 to USB converter] (http://www.comsol.com.au/Products-by-Category/USB-Converters/USB2-DB9M-02)
* Some flavour of Linux (Tested on Raspberry Pi)

### Building

In order to build this project you will need:

* build-essentials (or cmake)
* g++

### Setup

* Run ```{r, engine='bash', count_lines} cmake . ```
* run ```{r, engine='bash', count_lines}  make  ```
* Install [Logstash](https://www.elastic.co/products/logstash)
* Update /etc/logstash/logstash.conf, placing the contents of conf/logstash/logstash.conf in it.
* Install [Elastisearch](https://www.elastic.co/products/elasticsearch)
* Install [Kibana](https://www.elastic.co/products/kibana)
* Start Logstash, Elastiserch and Kibana.
* Create an ini file called jfyconfig.ini within the project directory. It supports three parameters within a [settings] section
    * device - Which is the device the USB converter is connected to
    * logstashHost - Hostname or IP address of the logstash instance
    * logstashPort - Port of the logstash instance
* Run ```{r, engine='bash', count_lines} bin/jfycron ``` as root to start sending to logstash.
* Browse to your logstash (e.g http://localhost:5601) to see your logs.
* An example Kibana dashboard configuration that can be imported is here: conf/kibana/kibana_dashboard.json


