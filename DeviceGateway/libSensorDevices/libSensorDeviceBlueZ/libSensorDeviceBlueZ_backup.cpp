/*=================================================================================
Basic explanation:

    Device Gateway cpp code

Authors: Daniel Denkovski (UKIM) (danield@feit.ukim.edu.mk)

Revision History:
    01.07.2015, ver 0.1
====================================================================================*/

#include "libSensorDeviceBlueZ.h"
#include "time_funcs.h"
#include "json_spirit_dgw.h"
#include <pthread.h>
#include <unistd.h>
#include <time.h>
#include "CAdataAdaptor.h"

static ISensorDevices* mysmBlueZ;

extern "C"
{
	#include <stdio.h>
	#include <string.h>
	#include <stdlib.h>
	#include <unistd.h>
	#include <sys/socket.h>
	#include <netinet/in.h>
	#include <glib.h>
	#include <gio/gio.h>
	#include <ieee11073.h>
	#include "src/communication/plugin/bluez/plugin_bluez.h"
	#include "src/communication/plugin/usb/plugin_usb.h"
	#include "src/communication/plugin/bluez/plugin_glib_socket.h"
	#include "src/communication/plugin/plugin_pthread.h"
	#include "src/util/log.h"
	#include "src/util/linkedlist.h"
	#include "src/communication/service.h"
	#include "src/dim/pmstore_req.h"
	#include "healthd_service.h"
	#include "healthd_common.h"
	#include "healthd_ipc_dbus.h"
	#include "healthd_ipc_tcp.h"
	#include "healthd_ipc_auto.h"
}

void read_entries(vector<measurement> *allMeas, vector<sensor> *sensors, string moteID, DataEntry *values, int size);

void process_data_entry(vector<measurement> *allMeas, vector<sensor> *sensors, string moteID,  DataEntry *data)
{
	long long time = getMillis();
	if (data != NULL && data->meta_data.size > 0 && data->meta_data.values != NULL) {
		int i = 0;
		bool hasMeta = false;
		char* sensorname;
		char* sensorunit;
		sensorType stype;
		
		for (i = 0; i < data->meta_data.size; i++) {
			MetaAtt *meta = &data->meta_data.values[i];
			if (meta != NULL && meta->name != NULL) {
				int metavalue = atoi(meta->value);
				sensorname = getCASensorName(metavalue);
				if (strcmp(sensorname,"") != 0)
				{
					stype = (sensorType)metavalue;
					sensorunit = getCASensorUnit(metavalue);
					printf("%s in %s\n", sensorname, sensorunit);
					hasMeta = true;
				}
			}
		}

		if (data->choice == SIMPLE_DATA_ENTRY) {
			SimpleDataEntry *simple = &data->u.simple;
			if (!simple->name || !simple->type || !simple->value) {
			// A malformed message might generate empty Data Entries
				return;
			}
			printf("value = %s\n", simple->value);
			measurement newmeas = measurement(moteID, stype, atof(simple->value), time);
			allMeas->push_back(newmeas);
			sensor newsens = sensor(stype, moteID, string(sensorunit), 0, 0, 0);
			sensors->push_back(newsens);
			//describe_simple_entry(&data->u.simple, sb);
		} else if (data->choice == COMPOUND_DATA_ENTRY) {
			CompoundDataEntry *cmp = &data->u.compound;
			if (!cmp->name || !cmp->entries) {
				// A malformed message might generate empty Data Entries
				return;
			}			
			//describe_cmp_entry(&data->u.compound, sb);
			read_entries(allMeas, sensors, moteID, cmp->entries, cmp->entries_count);
		}
	}
}

void read_entries(vector<measurement> *allMeas, vector<sensor> *sensors, string moteID, DataEntry *values, int size)
{
	int i = 0;

	for (i = 0; i < size; i++) {
		process_data_entry(allMeas, sensors, moteID, &values[i]);
	}
}

void readMeasurementsFromCADataList(vector<measurement> *allMeas, vector<sensor> *sensors, string moteID, DataList *list)
{
	if (list != NULL && list->values != NULL) {
		read_entries(allMeas, sensors, moteID, list->values, list->size);
	}
}

extern "C"
{
	static const int DBUS_SERVER = 0;
	static const int TCP_SERVER = 1;
	static const int AUTOTESTING = 2;

	healthd_ipc ipc;

	void new_data_received(Context *ctx, DataList *list)
	{
		DEBUG("Medical Device System Data");

		char *data = xml_encode_data_list(list);
		vector<measurement> newMeasurements;
		vector<sensor> newSensors;
		char moteIDchars[64];
		sprintf(&moteIDchars[0], "CA_mote-%d-%llu", ctx->id.plugin, ctx->id.connid);
        	string moteID(moteIDchars);
		readMeasurementsFromCADataList(&newMeasurements, &newSensors, moteID, list);

		if (mysmBlueZ->getSensors(moteID).size() == 0)
		{
			mysmBlueZ->deleteSensorMote(moteID);
			char dataIDchars[ctx->mds->system_id.length*2 + 1];
			for (int i=0; i<ctx->mds->system_id.length; i++)
				sprintf(&dataIDchars[i*2], "%02X", ctx->mds->system_id.value[i]);
			string dataID(dataIDchars);
			moteDetails newMote;
			int numSens = newMeasurements.size();
			newMote.smoteInfo = moteInfo(moteID, 0, string("blueZ"), string(""), dataID, location(0,0,0), string("wearable"), numSens);
			newMote.sensorsInfo = newSensors;
			mysmBlueZ->addSensorMote(newMote);			
		}

		if (newMeasurements.size() > 0)
			mysmBlueZ->pushMeasurements (moteID, newMeasurements);

		if (data) {
			ipc.call_agent_measurementdata(ctx->id, data);
			free(data);
		}
	}

	typedef struct {
		ContextId id;
		int handle;
		int instnumber;
		DataList *list;
	} segment_data_evt;

	static void segment_data_received_phase2(void *revt)
	{
		segment_data_evt *evt = (segment_data_evt *)revt;

		DEBUG("PM-Segment Data phase 2");

		char *data = xml_encode_data_list(evt->list);

		if (data) {
			ipc.call_agent_segmentdata(evt->id, evt->handle, evt->instnumber, data);
			free(data);
		}

		data_list_del(evt->list);
		free(revt);
	}

	void segment_data_received(Context *ctx, int handle, int instnumber, DataList *list)
	{
		DEBUG("PM-Segment Data");
		segment_data_evt *evt = (segment_data_evt *)calloc(1, sizeof(segment_data_evt));

		// Different from other callback events, "list" is not freed by core, but
		// it is passed ownership instead.

		// Encoding a whole PM-Segment to XML may take a *LONG* time. If the program
		// is single-threaded, encoding here would block the 11073 stack, causing
		// the agent to abort because it didn't get confirmation in time.

		// So, encoding XML from data list is better left to a thread, or, at very
		// least, delayed until there are no pending events.

		evt->id = ctx->id;
		evt->handle = handle;
		evt->instnumber = instnumber;
		evt->list = list;

		healthd_idle_add((void*)segment_data_received_phase2, evt);
	}

	void device_associated(Context *ctx, DataList *list)
	{
		DEBUG("Device associated");

		char *data = xml_encode_data_list(list);

		char moteIDchars[64];
		sprintf(&moteIDchars[0], "CA_mote-%d-%llu", ctx->id.plugin, ctx->id.connid);
        	string moteID(moteIDchars);
		mysmBlueZ->deleteSensorMote(moteID);
		char dataIDchars[ctx->mds->system_id.length*2 + 1];
		for (int i=0; i<ctx->mds->system_id.length; i++)
			sprintf(&dataIDchars[i*2], "%02X", ctx->mds->system_id.value[i]);
		string dataID(dataIDchars);
		moteDetails newMote;
		newMote.smoteInfo = moteInfo(moteID, 0, string("blueZ"), string(""), dataID, location(0,0,0), string("wearable"), 0);
		vector<sensor> newSensors;
		newMote.sensorsInfo = newSensors;
		mysmBlueZ->addSensorMote(newMote);

		if (data) {
			ipc.call_agent_associated(ctx->id, data);
			free(data);
		}
	}

	int device_connected(Context *ctx, const char *low_addr)
	{
		DEBUG("Device connected");
		ipc.call_agent_connected(ctx->id, low_addr);
		return 1;
	}

	int device_disconnected(Context *ctx, const char *low_addr)
	{
		DEBUG("Device disconnected");
		ipc.call_agent_disconnected(ctx->id, low_addr);
		return 1;
	}

	void device_disassociated(Context *ctx)
	{
		DEBUG("Device unassociated");
		char moteIDchars[64];
		sprintf(&moteIDchars[0], "CA_mote-%d-%llu", ctx->id.plugin, ctx->id.connid);
        	string moteID(moteIDchars);
		mysmBlueZ->deleteSensorMote(moteID);		
		ipc.call_agent_disassociated(ctx->id);
	}

	static void device_reqmdsattr_callback(Context *ctx, Request *r, DATA_apdu *response_apdu)
	{
		DEBUG("Medical Device Attributes");

		DataList *list = manager_get_mds_attributes(ctx->id);

		if (list) {
			char *data = xml_encode_data_list(list);
			if (data) {
				ipc.call_agent_deviceattributes(ctx->id, data);
				free(data);
			}
			data_list_del(list);
		}
	}

	void device_reqmdsattr(ContextId ctx)
	{
		DEBUG("device_reqmdsattr");
		manager_request_get_all_mds_attributes(ctx, device_reqmdsattr_callback);
	}

	void device_getconfig(ContextId ctx, char** xml_out)
	{
		DataList *list;

		DEBUG("device_getconfig");
		list = manager_get_configuration(ctx);

		if (list) {
			*xml_out = xml_encode_data_list(list);
			data_list_del(list);
		} else {
			*xml_out = strdup("");
		}
	}

	void device_reqmeasurement(ContextId ctx)
	{
		DEBUG("device_reqmeasurement");
		manager_request_measurement_data_transmission(ctx, NULL);
	}

	static void device_set_time_cb(Context *ctx, Request *r, DATA_apdu *response_apdu)
	{
		DEBUG("device_set_time_cb");
	}

	void device_set_time(ContextId ctx, unsigned long long time)
	{
		DEBUG("device_set_time");
		manager_set_time(ctx, time, device_set_time_cb);
	}

	void device_reqactivationscanner(ContextId ctx, int handle)
	{
		DEBUG("device_reqactivationscanner");
		manager_set_operational_state_of_the_scanner(ctx, (ASN1_HANDLE) handle, os_enabled, NULL);
	}

	void device_reqdeactivationscanner(ContextId ctx, int handle)
	{
		DEBUG("device_reqdeactivationscanner");
		manager_set_operational_state_of_the_scanner(ctx, (ASN1_HANDLE) handle, os_disabled, NULL);
	}

	void device_releaseassoc(ContextId ctx)
	{
		DEBUG("device_releaseassoc");
		manager_request_association_release(ctx);
	}

	void device_abortassoc(ContextId ctx)
	{
		DEBUG("device_abortassoc");
		manager_request_association_release(ctx);
	}

	static void device_get_pmstore_cb(Context *ctx, Request *r, DATA_apdu *response_apdu)
	{
		PMStoreGetRet *ret = (PMStoreGetRet*) r->return_data;
		DataList *list;
		char *data;

		DEBUG("device_get_pmstore_cb");

		if (!ret)
			return;

		if (ret->error) {
			// some error
			ipc.call_agent_pmstoredata(ctx->id, ret->handle, (char*)(""));
			return;
		}

		if ((list = manager_get_pmstore_data(ctx->id, ret->handle))) {
			if ((data = xml_encode_data_list(list))) {
				ipc.call_agent_pmstoredata(ctx->id, ret->handle, data);
				free(data);
				data_list_del(list);
			}
		}
	}

	void device_get_pmstore(ContextId ctx, int handle, int* ret)
	{
		DEBUG("device_get_pmstore");
		Request *r = manager_request_get_pmstore(ctx, handle, device_get_pmstore_cb);
		*ret = 0;
		if (!r) {
			DEBUG("no pmstore with handle %d", handle);
			*ret = 1;
		}
	}

	static void device_get_segminfo_cb(Context *ctx, Request *r, DATA_apdu *response_apdu)
	{
		PMStoreGetSegmInfoRet *ret = (PMStoreGetSegmInfoRet*) r->return_data;
		DataList *list;
		char *data;

		if (!ret)
			return;

		if ((list = manager_get_segment_info_data(ctx->id, ret->handle))) {
			if ((data = xml_encode_data_list(list))) {
				ipc.call_agent_segmentinfo(ctx->id, ret->handle, data);
				free(data);
				data_list_del(list);
			}
		}
	}

	static void device_get_segmdata_cb(Context *ctx, Request *r, DATA_apdu *response_apdu)
	{
		PMStoreGetSegmDataRet *ret = (PMStoreGetSegmDataRet*) r->return_data;

		if (!ret)
			return;

		ipc.call_agent_segmentdataresponse(ctx->id, ret->handle, ret->inst, ret->error);
	}

	static void device_clear_segm_cb(Context *ctx, Request *r, DATA_apdu *response_apdu)
	{
		PMStoreClearSegmRet *ret = (PMStoreClearSegmRet*) r->return_data;

		if (!ret)
			return;

		ipc.call_agent_segmentcleared(ctx->id, ret->handle, ret->inst, ret->error);
	}

	void device_get_segminfo(ContextId ctx, int handle, int* ret)
	{
		Request *req;

		DEBUG("device_get_segminfo");
		req = manager_request_get_segment_info(ctx, handle, device_get_segminfo_cb);
		*ret = req ? 0 : 1;
	}

	void device_get_segmdata(ContextId ctx, int handle, int instnumber, int* ret)
	{
		Request *req;

		DEBUG("device_get_segmdata");
		req = manager_request_get_segment_data(ctx, handle, instnumber, device_get_segmdata_cb);
		*ret = req ? 0 : 1;
	}

	void device_clearsegmdata(ContextId ctx, int handle, int instnumber,int *ret)
	{
		Request *req;

		DEBUG("device_clearsegmdata");
		req = manager_request_clear_segment(ctx, handle, instnumber, device_clear_segm_cb);
		*ret = req ? 0 : 1;
	}

	void device_clearallsegmdata(ContextId ctx, int handle, int *ret)
	{
		Request *req;

		DEBUG("device_clearsegmdata");
		req = manager_request_clear_segments(ctx, handle, device_clear_segm_cb);
		*ret = req ? 0 : 1;
	}

	static void timer_reset_timeout(Context *ctx)
	{
		if (ctx->timeout_action.id) {
			g_source_remove(ctx->timeout_action.id);
		}
	}

	static gboolean timer_alarm(gpointer data)
	{
		DEBUG("timer_alarm");
		Context *ctx = (Context *)data;
		void (*f)(void *) = (void (*)(void *))(ctx->timeout_action.func);
		if (f)
			f(ctx);
		return FALSE;
	}

	static int timer_count_timeout(Context *ctx)
	{
		ctx->timeout_action.id = g_timeout_add(ctx->timeout_action.timeout * 1000, timer_alarm, ctx);
		return ctx->timeout_action.id;
	}

	static gboolean healthd_idle_cb(gpointer d)
	{
		void **data = (void **)d;
		void (*f)(void*) = (void (*)(void *))data[0];
		void *param = (void *)data[1];
		f(param);
		free(data);
		return FALSE;
	}

	void healthd_idle_add(void *f, void *param)
	{
		void **data = (void **)malloc(2 * sizeof(void*));
		data[0] = f;
		data[1] = param;
		g_idle_add(&healthd_idle_cb, data);
	}

	void hdp_types_configure(uint16_t hdp_data_types[])
	{
		plugin_bluez_update_data_types(TRUE, hdp_data_types); // TRUE=sink
	}

	static GMainLoop *mainloop = NULL;

	static void app_clean_up()
	{
		g_main_loop_unref(mainloop);

		ipc.stop();
	}

	static void app_finalize(int sig)
	{
		DEBUG("Exiting with signal (%d)", sig);
		g_main_loop_quit(mainloop);
	}

	static void app_setup_signals()
	{
		signal(SIGINT, app_finalize);
		signal(SIGTERM, app_finalize);
	}
}

pthread_t hthreadBlueZ;

void *threadRunBlueZLoop(void *object)
{
	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_ref(mainloop);
	g_main_loop_run(mainloop);
	DEBUG("Main loop stopped");
	/*manager_finalize();
	DEBUG("manager_finalized");
	app_clean_up();
	DEBUG("Stopped.");*/
}

inline ISensorDevices::~ISensorDevices()
{
}

SensorDevicesBlueZ::~SensorDevicesBlueZ()
{
}

SensorDevicesBlueZ::SensorDevicesBlueZ()
{
	mysmBlueZ = this;
}

int SensorDevicesBlueZ::stopBlueZ()
{
	app_finalize(SIGTERM);
	pthread_join(hthreadBlueZ,NULL);
	//manager_stop();
	manager_finalize();
	DEBUG("manager_finalized");
	app_clean_up();
	DEBUG("Stopped.");
	return 1;
}

int SensorDevicesBlueZ::initializeDevice(vector<moteDetails> sensorMotes)
{
	/*int ret = initialize(sensorMotes);

	if (ret == 1)
	{
		cout << "SensorDevices: Initialization success. " << sensorMotes.size() << " sensor motes registered!\n";
	}
	else
	{
		cout << "SensorDevices: Initialization failed!\n";
		return 0;
	}*/

	app_setup_signals();
	#if !GLIB_CHECK_VERSION(2, 35, 0)
		g_type_init();
	#endif

	CommunicationPlugin bt_plugin;
	CommunicationPlugin usb_plugin;
	CommunicationPlugin tcp_plugin;

	CommunicationPlugin *plugins[] = {0, 0, 0};
	int plugin_count = 0;
	int usb_support = 0;
	int tcpp_support = 1;

	int i;

	int opmode = DBUS_SERVER;//AUTOTESTING;//TCP_SERVER;
	if (opmode == DBUS_SERVER) {
		healthd_ipc_dbus_init(&ipc);
	} else if (opmode == TCP_SERVER) {
		healthd_ipc_tcp_init(&ipc);
	} else if (opmode == AUTOTESTING) {
		healthd_ipc_auto_init(&ipc);
	}

	bt_plugin = communication_plugin();
	usb_plugin = communication_plugin();
	tcp_plugin = communication_plugin();

	plugin_bluez_setup(&bt_plugin);
	//plugin_pthread_setup(&bt_plugin);
	bt_plugin.timer_count_timeout = timer_count_timeout;
	bt_plugin.timer_reset_timeout = timer_reset_timeout;
	plugins[plugin_count++] = &bt_plugin;

	if (usb_support) {
		plugin_usb_setup(&usb_plugin);
		//plugin_pthread_setup(&usb_plugin);
		usb_plugin.timer_count_timeout = timer_count_timeout;
		usb_plugin.timer_reset_timeout = timer_reset_timeout;
		plugins[plugin_count++] = &usb_plugin;

	}

	if (tcpp_support) {
		plugin_glib_socket_setup(&tcp_plugin, 1, 6024);
		//plugin_pthread_setup(&tcp_plugin);
		tcp_plugin.timer_count_timeout = timer_count_timeout;
		tcp_plugin.timer_reset_timeout = timer_reset_timeout;
		plugins[plugin_count++] = &tcp_plugin;
	}

	manager_init(plugins);
	
	ManagerListener listener;// = MANAGER_LISTENER_EMPTY;
	listener.measurement_data_updated = &new_data_received;
	listener.segment_data_received = &segment_data_received;
	listener.device_available = &device_associated;
	listener.device_unavailable = &device_disassociated;
	listener.device_connected = &device_connected;
	listener.device_disconnected = &device_disconnected;

	manager_add_listener(listener);

	manager_start();
	ipc.start();

	mainloop = g_main_loop_new(NULL, FALSE);
	g_main_loop_ref(mainloop);
	g_main_loop_run(mainloop);
	DEBUG("Main loop stopped");
	//pthread_create(&hthreadBlueZ, NULL, threadRunBlueZLoop, this);

	return 1;
}

int SensorDevicesBlueZ::startMeasuring (string sensorMoteID, float reportPeriod)
{
	/*int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod == 0)
		{	
			setRate(sensorMoteID,reportPeriod);
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " started measurements with period of " << getRate(sensorMoteID) << "!\n";
			//start periodic measurement on sensor mote with ID = mote.smoteInfo.sensorMoteID
			return 1;
		}
		else
		{
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " already measuring with period of " << repPeriod << "!\n";
			return 0;
		}
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";*/
	return 0;
}

int SensorDevicesBlueZ::reconfigure (string sensorMoteID, float reportPeriod)
{
	/*int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod > 0)
		{
			setRate(sensorMoteID,reportPeriod);
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " reconfigured measurement period to " << getRate(sensorMoteID) << "!\n";
			//reconfigure period on sensor mote with ID = mote.smoteInfo.sensorMoteID
			return 1;
		}
		else 
		{
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " still not started!\n";
			return 0;
		}
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";*/
	return 0;
}

int SensorDevicesBlueZ::stopMeasuring (string sensorMoteID)
{
	/*int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		float repPeriod = getRate(sensorMoteID);

		if (repPeriod > 0)
		{	
			setRate(sensorMoteID,0);
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " stopped measurements!\n";
			//stop periodic measurements on sensor mote with ID = mote.smoteInfo.sensorMoteID
			return 1;
		}
		else
		{
			cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " not started!\n";
			return 0;
		}
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";*/
	return 0;
}

int SensorDevicesBlueZ::getMeasurements (string sensorMoteID)
{
	int index = findSensorMoteByID(sensorMoteID);
	if (index >= 0)
	{
		moteDetails mote = getSensorMoteDetails(index);
		//get new measurements from sensor mote with ID = mote.smoteInfo.sensorMoteID
		//this is just an example
		vector<sensor>::iterator it;
		for (it = mote.sensorsInfo.begin(); it != mote.sensorsInfo.end(); it++)
		{
			//query for measurements from each sensor
		}
		cout << "SensorDevices: SensorMote with ID = " << mote.smoteInfo.sensorMoteID << " queries for measurements!\n";
		return 1;
	}
	cout << "SensorDevices: SensorMote with ID = " << sensorMoteID << " does not exist!\n";
	return 0;
}
