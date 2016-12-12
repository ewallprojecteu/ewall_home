#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <unistd.h>
#include <signal.h>
#include <time.h>
#include <sys/socket.h>
#include <bluetooth/bluetooth.h>
#include <bluetooth/rfcomm.h>
#include <pthread.h>

#include <ieee11073.h>
#include "communication/plugin/plugin_tcp_agent.h"
#include "specializations/pulse_oximeter.h"
#include "agent.h"

struct mds_system_data;
pthread_t hthreadRunRfcommRcv;
pthread_t hthreadMainLoop;

static int csock;
static int runRfcomm = 1;
static char *dest;

intu8 AGENT_SYSTEM_ID_VALUE[] = { 0x11, 0x33, 0x55, 0x77, 0x99, 0xbb, 0xdd, 0xff};

static float oximetry = 0;
static float beats = 0;
static float prev_oximetry = 0;
static float prev_beats = 0;

void *oximeter_event_report_cb()
{
	time_t now;
	struct tm nowtm;
	struct oximeter_event_report_data* data =
		malloc(sizeof(struct oximeter_event_report_data));

	time(&now);
	localtime_r(&now, &nowtm);

	data->beats = beats;//60.5 + random() % 20;
	data->oximetry = oximetry;//90.5 + random() % 10;
	data->century = nowtm.tm_year / 100 + 19;
	data->year = nowtm.tm_year % 100;
	data->month = nowtm.tm_mon + 1;
	data->day = nowtm.tm_mday;
	data->hour = nowtm.tm_hour;
	data->minute = nowtm.tm_min;
	data->second = nowtm.tm_sec;
	data->sec_fractions = 50;

	return data;
}

struct mds_system_data *mds_data_cb()
{
	struct mds_system_data* data = malloc(sizeof(struct mds_system_data));
	memcpy(&data->system_id, AGENT_SYSTEM_ID_VALUE, 8);
	return data;
}

static ContextId CONTEXT_ID = {1, 1};

static CommunicationPlugin comm_plugin = COMMUNICATION_PLUGIN_NULL;

void *threadRunMainLoop(void *object)
{
        agent_connection_loop(CONTEXT_ID);
	fprintf(stderr, "Main loop stopped\n");
        return NULL;
}

void *threadRunRfcommRcv()
{
	int status = -1;              
	
        while(runRfcomm)
        {
		if (status == 0)
		{
		        char buf[64] = { 0 };
		        memset(buf, 0, sizeof(buf));
		        int bytes_read = read(csock, buf, sizeof(buf));
			//fprintf(stderr, "bytes_read = %d\n", bytes_read);               
		        if (bytes_read == 4)
		        {
		                //fprintf(stderr, "received [%s]\n", buf);
		                uint8_t oximetryVal = *(uint8_t*) &buf[2];
		                oximetry = (float) (oximetryVal);
		                uint16_t beatsVal = (((buf[0] & 3) << 7) | (buf[1] & 127)) & 511;
		                beats = (float) beatsVal;
				if ((oximetry >= 70) && (oximetry <= 100) && (beats >= 18) && (beats <= 320))
				{
					if ((oximetry != prev_oximetry) || (beats != prev_beats))
						agent_send_data(CONTEXT_ID);
				}
				prev_oximetry = oximetry;
				prev_beats = beats;
		        }
		        else
		        {
		                //fprintf(stderr, "received [%s]\n", buf);
				if (bytes_read < 0)
				{
					fprintf(stderr, "Nonin disconnected, trying to connect again\n");
				 	status = -1;
				}
		        }
		}
		else 
		{			
			sleep(1);
        		struct sockaddr_rc addr = { 0 };
			csock = socket(AF_BLUETOOTH, SOCK_STREAM, BTPROTO_RFCOMM);
			addr.rc_family = AF_BLUETOOTH;
			addr.rc_channel = (uint8_t) 1;
			str2ba(dest, &addr.rc_bdaddr);
			status = connect(csock, (struct sockaddr *)&addr, sizeof(addr));
			if (status == 0)
			{
				fprintf(stderr, "Connected to Nonin Pulse Oximeter\n");
				char dfchange[] = { 0x02, 0x70, 0x02, 0x02, 0x08, 0x03 };
				int bytes = write(csock, dfchange, sizeof(dfchange));
				fprintf(stderr, "Sent %d bytes DF8 configuration message to Nonin\n", bytes);							
			}
			else 
				fprintf(stderr, "Failed to connect to Nonin Pulse Oximeter\n");			
		}
        }
	close(csock);
	fprintf(stderr, "Exiting RFCOMM reading data thread\n");        
        return NULL;
}

static void device_associated(Context *ctx)
{
	fprintf(stderr, " main: Associated\n");
}

static void device_unavailable(Context *ctx)
{
	fprintf(stderr, " main: Disasociated\n");
}

static void device_connected(Context *ctx, const char *addr)
{
	fprintf(stderr, "main: Connected\n");
	agent_associate(ctx->id);
}

static void print_help()
{
	printf("Utility tool to simulate IEEE 11073 agent\n\n"
	       "Usage: nonin_ieee_agent addr (e.g. 00:25:56:BE:06:CA)\n\n");
}

static void timer_reset_timeout(Context *ctx)
{
}

static int timer_count_timeout(Context *ctx)
{
	return 1;
}

static void tcp_mode()
{
	int port = 6024;
	CONTEXT_ID.plugin = 1;
	CONTEXT_ID.connid = port;
	plugin_network_tcp_agent_setup(&comm_plugin, port);
}

int main(int argc, char **argv)
{
	comm_plugin = communication_plugin();

	if (argc != 2) {
                fprintf(stderr, "ERROR: you need to provide bluetooth MAC address\n");
                fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
                print_help();
                exit(1);
	} else {
		dest = argv[1];
	}

	/*char delim[] = " :";
	int cAddr = 0;
	char *token;
	for (token = strtok(dest, delim); token; token = strtok(NULL, delim))
	{
		unsigned int pomAddr;
		sscanf(token, "%x", &pomAddr);
		AGENT_SYSTEM_ID_VALUE[cAddr] = (intu8)pomAddr;
		cAddr++;
		if (cAddr > 6)
		{
		        fprintf(stderr, "ERROR: you have error in the bluetooth MAC address\n");
		        fprintf(stderr, "Try `%s --help' for more information.\n", argv[0]);
		        print_help();
		        exit(1);
		}		
	}*/

	tcp_mode();

	fprintf(stderr, "\nIEEE 11073 Nonin Pulse Oximeter agent\n");

	comm_plugin.timer_count_timeout = timer_count_timeout;
	comm_plugin.timer_reset_timeout = timer_reset_timeout;

	CommunicationPlugin *plugins[] = {&comm_plugin, 0};
	void *event_report_cb;
	int specialization;

	fprintf(stderr, "Starting Pulse Oximeter Agent\n");
	event_report_cb = oximeter_event_report_cb;
	// change to 0x0191 if you want timestamps
	specialization = 0x0190;

	agent_init(plugins, specialization, event_report_cb, mds_data_cb);

	AgentListener listener = AGENT_LISTENER_EMPTY;
	listener.device_connected = &device_connected;
	listener.device_associated = &device_associated;
	listener.device_unavailable = &device_unavailable;

	agent_add_listener(listener);

	agent_start();

	pthread_create(&hthreadRunRfcommRcv, NULL, &threadRunRfcommRcv, NULL);
	pthread_create(&hthreadMainLoop, NULL, &threadRunMainLoop, NULL);

        while(1)
        {
        	int input = -1;
                fprintf(stderr, "Input 0 to exit\n");
                int output = scanf("%d", &input);
                if (input == 0)
                {
 	        	fprintf(stderr, "Application exiting (%d)\n", output);        
			runRfcomm = 0;
                        break;
                }
        }

        close(csock);
        pthread_join(hthreadRunRfcommRcv, NULL);
	agent_stop();
	agent_finalize();
	pthread_kill(hthreadMainLoop, SIGINT);

	return 0;
}

