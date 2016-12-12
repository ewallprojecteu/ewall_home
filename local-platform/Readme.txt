****************************************************
Local Platform - installation and usage instructions
****************************************************
Local Platform components: Remote Proxy, Local Data Manager, Local Reasoners
Last updated: 28.01.2016.


##########
Disclaimer
##########
This prototype software is one of the results of the research project eWALL. While eWALL consortium partners make every effort to deliver high quality software, we do not guarantee that our software is free from defects or errors. Our software is provided “as is," and you use the software at your own risk. We make no warranties as to performance, merchantability, fitness for a particular purpose, or any other warranties whether expressed or implied. No oral or written communication from or information provided by any eWALL consortium partner shall create a warranty. Under no circumstances shall any eWALL consortium partner contributing to this software be liable for direct, indirect, special, incidental, or consequential damages resulting from the use, misuse, or inability to use this software, even if eWALL consortium parters have been advised of the possibility of such damages.



#############
Prerequisites
#############

    - Java Runtime Environment 7 or higher installed (you can check it by typing java –version in console)
    - Apache Couch DB (http://couchdb.apache.org/) installed and running
    - Each home sensing environment deployment must have its own unique sensing environment ID and related credentials (username and password). These parameters are obtain from the region administrator who provisions related users data and designated home sensing environment data in eWALL Cloud using eWALL Portal before installation and configuration of software in home sensing environment

######################
Installation procedure
######################

    1. Download latest Local Platform deployment package (tar or zip) and unpack to any local folder (delete whole folder if it is update)
    (i.e. on Ubuntu recommended unpacked folder is /home/ewall/local-platform/)

    2. Edit config.properties file using any text editor and set proper sensing environment id and user credentials (obtained from region administrator during provisioning process (see Prerequisites section)) by editing following parameters:
        - eu.ewall.platform.remoteproxy.sensingenvironment.id
        - eu.ewall.platform.remoteproxy.system.username
        - eu.ewall.platform.remoteproxy.system.password

    3. Edit additional parameters in the same document if environment configuration is not standard (such as http proxy, couch database, and amqp parameters)


    4. If installing on Ubuntu you can use upstart script that will run Local Platform as a backed service and enable automatic start/stop upon system reboot
        - open local-platform.conf file and check and modify unpacked folder location in line 11 and 14 (if needed)
        - move local-platform.conf file to /etc/init/ folder

#####
Usage
#####

    - If on Ubuntu with deployed upstart script you can use:
        sudo service local-platform start (to start local platform)
        sudo service local-platform stop (to stop local platform (*wait few seconds for graceful shut-down))
        sudo service local-platform status(to get status of local platform)
        sudo service local-platform restart (to restart local platform)

    - To manually start Local Platform you can use following command (stop Local Platform using Ctrl+C, and wait few seconds for graceful shut-down (all listener threads need to stop))):
        linux: sh start-local-platform.sh
        windows: start-local-platform.bat


#######
Logging
#######

	- Log files of Local Platform can be found in {unpacked_folder}/log folder (i.e. /home/ewall/local-platform/logs)
