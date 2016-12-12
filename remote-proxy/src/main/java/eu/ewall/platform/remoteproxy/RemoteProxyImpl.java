/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy;

import java.net.URI;
import java.util.Properties;

import org.apache.commons.configuration.PropertiesConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import eu.ewall.gateway.lightcontrol.PHHueInitializer;
import eu.ewall.platform.commons.datamodel.ewallsystem.PointOfContact;
import eu.ewall.platform.commons.datamodel.ewallsystem.ProxyStatus;
import eu.ewall.platform.commons.datamodel.ewallsystem.RegistrationStatus;
import eu.ewall.platform.remoteproxy.api.MethodResult;
import eu.ewall.platform.remoteproxy.api.RemoteProxyDBProvider;
import eu.ewall.platform.remoteproxy.api.RemoteProxyProvider;
import eu.ewall.platform.remoteproxy.api.RequestDescription;
import eu.ewall.platform.remoteproxy.api.ResponseDescription;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;
import eu.ewall.platform.remoteproxy.comm.http.RPRestHttpClient;
import eu.ewall.platform.remoteproxy.config.RemoteProxyConfig;
import eu.ewall.platform.remoteproxy.config.RemoteProxyConfigFile;
import eu.ewall.platform.remoteproxy.services.LongPollService;
import eu.ewall.platform.remoteproxy.services.PeriodicAnnouncingService;

/**
 * The Class RemoteProxyImpl.
 *
 * @author emirmos
 */
public class RemoteProxyImpl implements RemoteProxyProvider {

	/** The logger. */
	Logger logger = LoggerFactory.getLogger(RemoteProxyImpl.class);

	/** The e wall system root. */
	private String eWallSystemRoot;

	/** The sensing environment id. */
	private String sensingEnvironmentId;

	/** The type of communcation. */
	private String connectionType;

	/** The rest client. */
	private RPCloudClient restClient;

	/** The contactURI. */
	private URI contactURI;

	/** The status. */
	private ProxyStatus status = ProxyStatus.NOT_INITIALIZED;

	/** The registration status. */
	private RegistrationStatus registrationStatus = RegistrationStatus.NOT_REGISTERED;

	/** The config manager. */
	private RemoteProxyConfig configManager;

	/** The long polling service. */
	private LongPollService longPollService;

	/** The periodic announce service. */
	private PeriodicAnnouncingService periodicAnnouncingService;

	/** The username. */
	private String username;

	/** The password. */
	private String password;

	/** The props. */
	private PropertiesConfiguration props;

	private RemoteProxyDBProvider remoteProxyDBProvider;

	/**
	 * Instantiates a new remote proxy impl.
	 */
	public RemoteProxyImpl() {
		super();
	}

	/**
	 * Initialize.
	 *
	 * @return true, if successful
	 */
	public boolean initialize() {
		configManager = new RemoteProxyConfigFile();
		props = configManager.getProperties();

		if (props == null) {
			logger.error("Error in loading configuration properties. Initialization failed");
			status = ProxyStatus.FAILURE;
			return false;
		}

		eWallSystemRoot = props.getString("eu.ewall.platform.remoteproxy.root");

		if (!eWallSystemRoot.endsWith("/"))
			eWallSystemRoot = eWallSystemRoot.concat("/");

		logger.info("eWALL cloud root url: {}", eWallSystemRoot);

		sensingEnvironmentId = props
				.getString("eu.ewall.platform.remoteproxy.sensingenvironment.id");
		logger.info("Sensing envrionment ID: {}", sensingEnvironmentId);

		// return if required properties are missing
		if (eWallSystemRoot == null || sensingEnvironmentId == null) {
			logger.error("Not all required configuration paramaters were set during initialization.");
			status = ProxyStatus.FAILURE;
			return false;
		}

		connectionType = props.getString("eu.ewall.platform.remoteproxy.comm");

		if (connectionType.equals("http")) {
			
			// set proxy to system properties
			Properties systemProperties = System.getProperties();
			String proxyHost = props.getString("http.proxyHost");
			String proxyPort = props.getString("http.proxyPort");

			if (proxyHost != null && proxyPort != null && !proxyHost.isEmpty()
					&& !proxyPort.isEmpty()) {
				systemProperties.put("http.proxyHost",
						props.getProperty("http.proxyHost"));
				systemProperties.put("http.proxyPort",
						props.getProperty("http.proxyPort"));

				logger.info("Set HTTP proxy: {}:{}" + proxyHost, proxyPort);
			}
			
			restClient = new RPRestHttpClient(eWallSystemRoot,
					sensingEnvironmentId);

		} else {
			logger.error("Configuration paramaters for communication were not set during initialization. Please set eu.ewall.platform.remoteproxy.comm = http or amqp");
			status = ProxyStatus.FAILURE;
		}

		username = props
				.getString("eu.ewall.platform.remoteproxy.system.username");
		password = props
				.getString("eu.ewall.platform.remoteproxy.system.password");

		if (username == null || password == null) {
			logger.warn("Credential are not found in configuration properties");
		}

		if (!restClient.login(username, password)) {
			logger.error("Login failed for user {}.", username);
			return false;
		} 
			
		logger.info("Sucessfull logged in to eWALL cloud using username {}", username);

		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#doRegister()
	 */
	@Override
	public boolean doRegister() {

		if (sensingEnvironmentId == null) {
			logger.warn("sensingEnvironmentId is not set. Unsucessfull registration.");
			return false;
		}

		PointOfContact poc = new PointOfContact();
		poc.setContactURI(contactURI);
		poc.setProxyStatus(status);
		poc.setCreationTime(System.currentTimeMillis());
		poc.setLastModifiedTime(System.currentTimeMillis());

		String localPlatformVersion = this.getEWallHomeSoftwareVersion();

		poc.setLocalPlatformVersion(localPlatformVersion);

		MethodResult registrationResult = restClient.register(poc);

		if (registrationResult == MethodResult.SUCCESS) {
			logger.info("Succesfully registerd sensing environemnt {} at eWall system {}", sensingEnvironmentId, eWallSystemRoot);
			
			this.registrationStatus = RegistrationStatus.REGISTERED;
			return true;

		} else if (registrationResult == MethodResult.NOT_AUTHORIZED) {
			if (restClient.login()) {
				registrationResult = restClient.register(poc);

				if (registrationResult == MethodResult.SUCCESS) {
					logger.info("Succesfully registerd sensing environemnt {} at eWall system {}", sensingEnvironmentId, eWallSystemRoot);
					this.registrationStatus = RegistrationStatus.REGISTERED;
					return true;

				}

			}

		}

		if (registrationResult == MethodResult.NOT_ALLOWED) {
			logger.info("Unsuccesfull registration of sensing environemnt {} at eWall system {}. Sensing environment is disabled at eWALL cloud", sensingEnvironmentId, eWallSystemRoot);
			this.registrationStatus = RegistrationStatus.NOT_REGISTERED;

			/* Shutdowns local-platform if sensing environment is disabled */
			System.exit(0);
			return false;

		} else if (registrationResult == MethodResult.NOT_REQUIRED_VERSION) {
			logger.info("Unsuccesfull registration of sensing environemnt {} at eWall system {}. eWALL home software version ({}) is older than minimum requred by eWALl cloud", sensingEnvironmentId, eWallSystemRoot, localPlatformVersion);
			
			this.registrationStatus = RegistrationStatus.NOT_REGISTERED;

			/*
			 * Shutdowns local-platform if local-platform version not equals to
			 * required one in CGW props
			 */
			System.exit(0);
			return false;

		} else {
			logger.info("Unsuccesfull registration of sensing environemnt {} at eWall system {}. Result code = {}", sensingEnvironmentId, eWallSystemRoot, registrationResult);
			
			this.registrationStatus = RegistrationStatus.NOT_REGISTERED;
			return false;
		}
	}

	private String getEWallHomeSoftwareVersion() {
		if (remoteProxyDBProvider != null) {
			remoteProxyDBProvider.intializeDBClient(props);
			String version = remoteProxyDBProvider.getVersion();
			logger.debug("eWall home software version {} obtained from local database", version);
			return version;
		}
		logger.warn("Remote Proxy DB provider is not intialized. Returning null for eWall home version");
		return null;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#doRequest(eu.ewall
	 * .platform.remoteproxy.api.RequestDescription)
	 */
	@Override
	public ResponseDescription doRequest(RequestDescription request) {
		logger.debug("doRequest() called with paramater request = " + request);
		logger.info("doRequest() not implemented yet");
		return null;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.api.RemoteProxyNativeProvider#pushData(
	 * java.lang.String, java.lang.String)
	 */
	@Override
	public boolean pushData(String deviceId, String content) {

		logger.debug("Pushing data device {} with content {}.", deviceId, content);

		if (registrationStatus != RegistrationStatus.REGISTERED) {
			logger.warn("Sensing environment {} is not registered. Unable to send data to cloud.", sensingEnvironmentId);
			return false;
		}

		if (sensingEnvironmentId == null) {
			logger.warn("sensingEnvironmentId is not set.");
			return false;
		}

		boolean doCreateSuccessful = restClient.doPost(deviceId, content);

		if (doCreateSuccessful) {
			logger.debug("Succesfully added new content for device ", deviceId);
			return true;
		} 
		
		logger.warn("Could not add new content for device {}.", deviceId);
		return false;
		
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.api.RemoteProxyNativeProvider#updateData
	 * (java.lang.String, java.lang.String, java.lang.String)
	 */
	@Override
	public boolean updateData(String deviceId, String contentId, String content) {

		logger.debug("Updating data for device {} with content {}.", deviceId, content);

		if (registrationStatus != RegistrationStatus.REGISTERED) {
			logger.info("Sensing environment is not registered. Unable to send data to cloud.");
			return false;
		}

		if (sensingEnvironmentId == null) {
			logger.warn("sensingEnvironmentId is null.");
			return false;
		}

		boolean doUpdateSuccessful = restClient.doPut(deviceId, contentId,
				content);

		if (doUpdateSuccessful) {
			logger.debug("Succesfully updated content (id {}) for device (id {})", contentId, deviceId);
			this.status = ProxyStatus.ONLINE;
			return true;
		} 
			
		return false;
		
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#doPublish(java.
	 * lang.String, java.lang.String)
	 */
	@Override
	public boolean doPublish(String deviceId, String content) {
		logger.debug("Publishing content for device {} with content {}", deviceId, content);

		if (registrationStatus != RegistrationStatus.REGISTERED) {
			logger.info("Sensing environment is not registered. Unable to send data to cloud.");
			return false;
		}

		if (sensingEnvironmentId == null) {
			logger.warn("sensingEnvironmentId is null.");
			return false;
		}

		boolean doCreateSuccessful = restClient.doPost(deviceId, content);

		if (doCreateSuccessful) {
			logger.info("Succesfully added new content for device {}", deviceId);
			return true;
		} 

		return false;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#doSubscribe(java
	 * .lang.String, java.lang.String)
	 */
	@Override
	public boolean doSubscribe(String remoteProxyConsumerId,
			String subscribeRequest) {
		// TODO Auto-generated method stub
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#startup()
	 */
	@Override
	public boolean startup() {

		if (!initialize()) {
			return false;
		}

		if (status == ProxyStatus.ONLINE) {
			logger.info("Sensing environment {} is already online. Startup unsuccessful.", sensingEnvironmentId);
			return false;
		}

		status = ProxyStatus.ONLINE;

		if (!doRegister()) {
			return false;
		}

		/*
		 * Don't check if long polling service didn't started because it can be
		 * manually disabled via properties file
		 */
		startLongPolling();

		/*
		 * Note: remote-proxy does NOT depend on successfully initializing
		 * PHHueLightController
		 */
		initPHHueLightController();

		if (!startAnnounceService()) {
			return false;
		}

		return true;
	}

	/**
	 * Inits the ph hue light controller.
	 */
	private void initPHHueLightController() {
		try {
			PHHueInitializer.initialize();
		} catch (Exception e) {
			logger.warn("Philips Hue failed to initialized: ", e.getMessage());
		}
	}

	/**
	 * Start periodic announcing.
	 *
	 * @return true, if successful
	 */
	boolean startAnnounceService() {

		if (status != ProxyStatus.ONLINE) {
			logger.warn("Proxy is not online. Periodic announcing service was not started.");
			return false;
		}

		if (registrationStatus != RegistrationStatus.REGISTERED) {
			logger.warn("Proxy is not registered. Periodic announcing service was not started.");
			return false;
		}

		periodicAnnouncingService = new PeriodicAnnouncingService(this);
		periodicAnnouncingService.startAnnouncing();

		logger.info("Successfully started periodic announcing service.");
		return true;
	}

	/**
	 * Start long polling.
	 *
	 * @return true, if successful
	 */
	boolean startLongPolling() {

		if (status != ProxyStatus.ONLINE) {
			logger.warn("Proxy is not online. Long polling service was not started.");
			return false;
		}

		if (registrationStatus != RegistrationStatus.REGISTERED) {
			logger.warn("Proxy is not registered. Long polling service was not started.");
			return false;
		}

		if (props != null) {
			String longPollingEnabledProperty = props
					.getString("eu.ewall.platform.remoteproxy.longPolling.enable");

			// don't start long polling thread if parameter is disabled
			if ("false".equals(longPollingEnabledProperty)) {
				logger.warn("Long polling service was not started because \"longPolling\" parameter is disabled.");
				return false;
			}

		} else {
			logger.warn("Configuration properties are null. Unable to start long pooling service.");
			return false;
		}

		String updateURL = eWallSystemRoot
				+ "cloud-gateway/sensingenvironments/" + sensingEnvironmentId
				+ "/getUpdate";
		longPollService = new LongPollService(updateURL, restClient, this);

		// start long polling to cloud gateway
		longPollService.startLongPolling();

		logger.info("Successfully started long polling service.");
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#shutdown()
	 */
	@Override
	public boolean shutdown() {

		if (this.status == ProxyStatus.ONLINE) {
			this.status = ProxyStatus.OFFLINE;
		}

		if (this.registrationStatus == RegistrationStatus.REGISTERED) {

			this.registrationStatus = RegistrationStatus.NOT_REGISTERED;
			if (restClient != null) {
				PointOfContact poc = new PointOfContact();
				poc.setProxyStatus(status);
				poc.setCreationTime(System.currentTimeMillis());
				poc.setLastModifiedTime(System.currentTimeMillis());
				poc.setExpirationTime(0);
				restClient.deRegister(poc);
			}
		}

		if (this.remoteProxyDBProvider != null)
			remoteProxyDBProvider.close();

		if (longPollService != null)
			longPollService.stopLongPolling();

		if (periodicAnnouncingService != null)
			periodicAnnouncingService.stopAnnouncing();

		logger.debug("RemoteProxy shutdown was successful.");
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#connect()
	 */
	@Override
	public boolean connect() {
		// TODO Auto-generated method stub
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#disconnect()
	 */
	@Override
	public boolean disconnect() {
		// TODO Auto-generated method stub
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#getStatus()
	 */
	@Override
	public ProxyStatus getStatus() {
		return status;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#
	 * setConfigurationProperites(java.util.Properties)
	 */
	@Override
	public boolean setConfigurationProperites(
			PropertiesConfiguration configrationProperties) {
		configManager.setProperties(configrationProperties);
		configManager.loadProperties();
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#
	 * getConfigurationProperites()
	 */
	@Override
	public PropertiesConfiguration getConfigurationProperites() {
		return configManager.getProperties();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#restart()
	 */
	@Override
	public boolean restart(PropertiesConfiguration oldWorkingProperties) {

		logger.info("Restarting remote proxy...");
		shutdown();

		// if remote proxy cant start with new properties, load old properties
		// and restart
		if (startup() == false) {
			logger.info("Restarting with new properties failed, restarting with backup (last working) properties...");
			setConfigurationProperites(oldWorkingProperties);

			if (startup() == false) {
				logger.warn("Cannot restart remote proxy with backup properties, stopping...");
				System.exit(1);
			}

			logger.info("Old (backup) remote proxy configuration successfully restored.");

			// if backup is restored, return false
			return false;
		}

		logger.info("Restarting succeeded.");

		// return true only if successfully restarted with new properties
		return true;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#getConfigManager()
	 */
	@Override
	public RemoteProxyConfig getConfigManager() {
		return this.configManager;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.api.RemoteProxyProvider#getRPCloudClient()
	 */
	@Override
	public RPCloudClient getRPCloudClient() {
		return this.restClient;
	}

	@Override
	public void setRemoteProxyDBProvider(
			RemoteProxyDBProvider remoteProxyDBProvider) {
		this.remoteProxyDBProvider = remoteProxyDBProvider;
	}

}
