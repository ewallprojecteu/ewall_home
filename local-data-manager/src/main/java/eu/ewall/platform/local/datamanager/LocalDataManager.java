/*******************************************************************************
 * Copyright 2015 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.local.datamanager;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.commons.datamodel.message.Notification;
import eu.ewall.platform.commons.datamodel.profile.preferences.SystemPreferences;
import eu.ewall.platform.local.datamanager.api.SensingDataListener;
import eu.ewall.platform.local.datamanager.config.LocalDataManagerConfig;
import eu.ewall.platform.local.datamanager.config.LocalDataManagerConfigFile;
import eu.ewall.platform.local.datamanager.config.PrivacyConfig;
import eu.ewall.platform.local.datamanager.controller.Controller;
import eu.ewall.platform.local.datamanager.couchdb.CouchDbListenerManager;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The Class LocalDataManager.
 * 
 * @author emirmos
 */
public class LocalDataManager {

	/** The logger. */
	Logger logger = LoggerFactory.getLogger(LocalDataManager.class);

	/** The rp couch db client. */
	private static CouchDbListenerManager couchDbListenerManager;
	
	/** The rest client. */
	private RPCloudClient restClient;
	
	/** The config manager. */
	private LocalDataManagerConfig configManager;

	/** The resume. */
	private boolean resume = false;
	
	/**  Privacy config file manager. */
	private LocalDataManagerConfig privacyConfig;
	
	/** The Constant controller. */
	private final static Controller controller = new Controller();
	
	/**
	 * Startup.
	 *
	 * @param restClient the rest client
	 * @return true, if successful
	 */
	public boolean startup(RPCloudClient restClient) {
		
		this.restClient = restClient;

		if (!syncAndStartDBNotificationListener())
			return false;
		return true;
	}

	/**
	 * Shutdown.
	 *
	 * @return true, if successful
	 */
	public boolean shutdown() {
		if (!stopDBNotificationListener())
			return false;
		
		return true;
	}
	
	
	/**
	 * Register listener.
	 *
	 * @param listener the listener
	 * @return true, if successful
	 */
	public boolean registerListener(SensingDataListener listener) {
		controller.addListener(listener);
		return true;
	}
	
	/**
	 * Deregister listener.
	 *
	 * @param listener the listener
	 * @return true, if successful
	 */
	public boolean deregisterListener(SensingDataListener listener) {
		controller.removeListener(listener);
		return true;
	}
	
	/**
	 * Initializes, synchronizes and starts continuous CouchDB notification listener.
	 *
	 * @return true, if successful
	 */
	private boolean syncAndStartDBNotificationListener() {

		List<Device> devices = restClient.getDeviceInfo();
		if (devices == null || devices.isEmpty()) {
			logger.warn("No device info found. Unsuccessfully start of local DB listener.");
			return false;
		}
		
		couchDbListenerManager = new CouchDbListenerManager(configManager, 
				privacyConfig, restClient, controller, devices, resume);

		couchDbListenerManager.sync();
		couchDbListenerManager.start();
		couchDbListenerManager.periodicallyStoreLastSentUpdateSeq();

		logger.info("Successfully started local DB notification listener.");

		return true;

	}

	/**
	 * Stops CouchDB notification listener.
	 *
	 * @return true, if successful
	 */
	private boolean stopDBNotificationListener() {

		logger.info("Stopping local DB notification listener...");
		if (couchDbListenerManager != null) {
			couchDbListenerManager.stopRunning();
		}
		return true;
	}

	/**
	 * Instantiates a new local data manager.
	 *
	 * @param resume the resume
	 */
	public LocalDataManager(boolean resume) {
		super();
		configManager = new LocalDataManagerConfigFile();
		privacyConfig = new PrivacyConfig();
		this.resume  = resume;
	}

	
	/**
	 * Process the notification.
	 *
	 * @param notification the notification
	 * @return true, if successful
	 */
	public boolean processNotification(Notification notification) {
		if (restClient != null) {
			restClient.doPostNotification(notification);
			return true;
		}
		logger.warn("Error in processing user notifications: restClient is null. Unsucessful notification processing.");
		return false;
	}
	

	
	/**
	 * Process caregiver notification.
	 *
	 * @param notification the notification
	 * @return true, if successful
	 */
	public boolean processCaregiverNotification(Notification notification) {
		if (restClient != null) {
			restClient.doPostCaregiverNotification(notification);
			return true;
		}
		logger.warn("Error in processing caregiver notifications: restClient is null. Unsucessful notification processing.");
		return false;
	}
	
	public SystemPreferences getUserSystemPreferences() {
		if (restClient != null) {
			return restClient.getUserSystemPreferences();
			
		}
		logger.warn("Error in getting user system preferneces: restClient is null. Unable to obtain user preferences.");
		return null;
	}
	
}
