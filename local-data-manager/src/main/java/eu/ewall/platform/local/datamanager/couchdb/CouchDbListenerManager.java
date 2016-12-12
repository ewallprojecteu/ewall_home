package eu.ewall.platform.local.datamanager.couchdb;

import java.util.ArrayList;
import java.util.List;

import org.apache.commons.configuration.PropertiesConfiguration;
import org.lightcouch.CouchDbClient;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;



import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.local.datamanager.config.LocalDataManagerConfig;
import eu.ewall.platform.local.datamanager.controller.Controller;
import eu.ewall.platform.remoteproxy.api.ContinueSince;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The Class CouchDbListenerManager.
 * 
 * @author emirmos
 */
public class CouchDbListenerManager {

	/** The logger. */
	Logger logger = LoggerFactory.getLogger(CouchDbListenerManager.class);

	/** The rp couch db listeners. */
	List<RPCouchDbListener> rpCouchDbListeners;

	/** The sequence storer thread. */
	private Thread sequenceStorerThread;

	private ContinueSince continueSince;

	/**
	 * Instantiates a new couch db listener manager.
	 *
	 * @param config
	 *            the config
	 * @param privacy
	 *            the privacy
	 * @param rpCloudClient
	 *            the rp cloud client
	 * @param controller
	 *            the controller
	 * @param devices 
	 * @param resume
	 *            the resume
	 */
	public CouchDbListenerManager(LocalDataManagerConfig config,
			LocalDataManagerConfig privacy, RPCloudClient rpCloudClient,
			Controller controller, List<Device> devices, boolean resume) {
		rpCouchDbListeners = new ArrayList<RPCouchDbListener>();

		PropertiesConfiguration properties = config.getProperties();

		String createDbIfNotExistString = properties
				.getString("couchdb.createdb.if-not-exist");
		String protocol = properties.getString("couchdb.protocol");
		String host = properties.getString("couchdb.host");
		String portString = properties.getString("couchdb.port");
		String username = properties.getString("couchdb.username");
		String password = properties.getString("couchdb.password");

		String continueSinceStr = properties
				.getString("eu.ewall.platform.remoteproxy.couchdb.continue-reading-since");
		if (continueSinceStr == null || continueSinceStr.isEmpty()) {
			continueSince = ContinueSince.LAST;
		} else {
			continueSince = ContinueSince.valueOf(continueSinceStr);
		}

		if (resume) {
			// regardless of config properties
			continueSince = ContinueSince.LAST_SENT;
			logger.info("Resume option set to true. Resuming from LAST_SENT local db entry.");
		}

		logger.info("continueSince set to {}", continueSince);

		boolean createDbIfNotExist = true;
		if (createDbIfNotExistString != null) {
			createDbIfNotExist = Boolean.valueOf(createDbIfNotExistString);
		}
		if (protocol == null) {
			protocol = "http";
		}
		if (host == null) {
			host = "127.0.0.1";
		}
		int port = 5984;
		if (portString != null) {
			port = Integer.parseInt(portString);
		}

		if (username != null) {
			if (username.length() == 0) {
				username = null;
			}
		}
		if (password != null) {
			if (password.length() == 0) {
				password = null;
			}
		}

		for (Device device : devices) {
			switch (device.getType()) {
			case ENVIRONMENTAL_SENSOR:
				String dbName = device.getName();
				try {
					CouchDbClient client = new CouchDbClient(dbName,
							createDbIfNotExist, protocol, host, port, username,
							password);

					RPCouchDbListener listener = new EnvironmentalSensingDBListener(
							client, privacy, rpCloudClient, controller, device, continueSince);
					rpCouchDbListeners.add(listener);
					logger.info("Listener for {} added.", dbName);
				} catch (Exception e) {
					logger.error("Error occured while adding listener for {} db", dbName, e);
				}
				break;

			case ACTIVITY_SENSOR:
				dbName = device.getName();
				try {
					CouchDbClient client = new CouchDbClient(dbName,
							createDbIfNotExist, protocol, host, port, username,
							password);

					RPCouchDbListener listener = new ActivitySensingDBListener(
							client, privacy, rpCloudClient, controller, device, continueSince);
					rpCouchDbListeners.add(listener);
					logger.info("Listener for {} added.", dbName);
				} catch (Exception e) {
					logger.error("Error occured while adding listener for {} db", dbName, e);
				}
				break;
				
			case FURNITURE_SENSOR:
				dbName = device.getName();
				try {
					CouchDbClient client = new CouchDbClient(dbName,
							createDbIfNotExist, protocol, host, port, username,
							password);

					RPCouchDbListener listener = new FurnitureSensingDBListener(
							client, privacy, rpCloudClient, controller, device, continueSince);
					rpCouchDbListeners.add(listener);
					logger.info("Listener for {} added.", dbName);
				} catch (Exception e) {
					logger.error("Error occured while adding listener for {} db", dbName, e);
				}
				break;

			case VISUAL_SENSOR:
				dbName = device.getName();
				try {
					CouchDbClient client = new CouchDbClient(dbName,
							createDbIfNotExist, protocol, host, port, username,
							password);

					RPCouchDbListener listener = new VisualSensingDBListener(
							client, privacy, rpCloudClient, controller, device, continueSince);
					rpCouchDbListeners.add(listener);
					logger.info("Listener for {} added.", dbName);
				} catch (Exception e) {
					logger.error("Error occured while adding listener for {} db", dbName, e);
				}
				break;
				
			case VITALS_SENSOR:
				dbName = device.getName();
				try {
					CouchDbClient client = new CouchDbClient(dbName,
							createDbIfNotExist, protocol, host, port, username,
							password);

					RPCouchDbListener listener = new VitalsSensingDBListener(
							client, privacy, rpCloudClient, controller, device, continueSince);
					rpCouchDbListeners.add(listener);
					logger.info("Listener for {} added.", dbName);
				} catch (Exception e) {
					logger.error("Error occured while adding listener for {} db", dbName, e);
				}
				break;

			case AUDIO_SENSOR:
				dbName = device.getName();
				try {
					CouchDbClient client = new CouchDbClient(dbName,
							createDbIfNotExist, protocol, host, port, username,
							password);

					RPCouchDbListener listener = new SpeakerSensingDBListener(
							client, privacy, rpCloudClient, controller, device, continueSince);
					rpCouchDbListeners.add(listener);
					logger.info("Listener for {} added.", dbName);
				} catch (Exception e) {
					logger.error("Error occured while adding listener for {} db", dbName, e);
				}
				break;
				
			case POWER_SENSOR:
				dbName = device.getName();
				try {
					CouchDbClient client = new CouchDbClient(dbName,
							createDbIfNotExist, protocol, host, port, username,
							password);

					RPCouchDbListener listener = new AppliancePowerSensingDBListener(
							client, privacy, rpCloudClient, controller, device, continueSince);
					rpCouchDbListeners.add(listener);
					logger.info("Listener for {} added.", dbName);
				} catch (Exception e) {
					logger.error("Error occured while adding listener for {} db", dbName, e);
				}
				break;
			default:
				break;
			}
		}

		if (rpCouchDbListeners.isEmpty()) {
			// exit
			logger.error("No database listeners added. Stopping local data manager.");
			this.stopRunning();
			System.exit(0);
		}
		

	}

	/**
	 * Gets the couch db listeners.
	 *
	 * @return the couch db listeners
	 */
	public List<RPCouchDbListener> getCouchDbListeners() {
		return rpCouchDbListeners;
	}

	/**
	 * Sync.
	 */
	public void sync() {
		if (continueSince == ContinueSince.LAST_SENT) {
			logger.info("continueSince set to LAST_SENT. Performing data sync with cloud.");
			for (RPCouchDbListener rpCouchDbListener : rpCouchDbListeners) {
				/* Logger exists in sync method */
				//TODO Temporarily disabled syncing.
				logger.info("Sync disabled.");
				//rpCouchDbListener.sync();
			}
		} else
			logger.info("continueSince not set to LAST_SENT. Skipping data sync with cloud.");

	}

	/**
	 * Start.
	 */
	public void start() {
		for (RPCouchDbListener rpCouchDbListener : rpCouchDbListeners) {
			rpCouchDbListener.start();
			logger.info("New db listener started: {} ", rpCouchDbListener.getDBName());
		}
	}

	/**
	 * Periodically store last sent update seq.
	 */
	public void periodicallyStoreLastSentUpdateSeq() {
		sequenceStorerThread = new Thread(new Runnable() {

			@Override
			public void run() {
				while (true) {
					int period = 1 * 60 * 60 * 1000; // 1 hour
					try {
						Thread.sleep(period);
					} catch (InterruptedException e) {
						logger.warn(e.getMessage());
						break;
					}

					for (RPCouchDbListener rpCouchDbListener : rpCouchDbListeners) {
						rpCouchDbListener
								.storeLastSentUpdateSeq(rpCouchDbListener
										.getDbClient().context().info()
										.getUpdateSeq());
					}
				}
			}
		});
		sequenceStorerThread.start();
	}

	/**
	 * Stop running.
	 */
	public void stopRunning() {
		for (RPCouchDbListener rpCouchDbListener : rpCouchDbListeners) {
			rpCouchDbListener.stopRunning();
			logger.info("Stopping db listener: "
					+ rpCouchDbListener.getDBName());
		}

		if (sequenceStorerThread != null) {
			sequenceStorerThread.interrupt();
		}
	}
}
