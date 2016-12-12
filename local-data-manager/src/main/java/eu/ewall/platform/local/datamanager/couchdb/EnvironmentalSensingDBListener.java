package eu.ewall.platform.local.datamanager.couchdb;

import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.commons.configuration.PropertiesConfiguration;
import org.lightcouch.CouchDbClient;
import org.slf4j.LoggerFactory;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.commons.datamodel.marshalling.json.DM2JsonObjectMapper;
import eu.ewall.platform.commons.datamodel.sensing.EnvironmentalSensing;
import eu.ewall.platform.local.datamanager.config.LocalDataManagerConfig;
import eu.ewall.platform.local.datamanager.controller.Controller;
import eu.ewall.platform.remoteproxy.api.ContinueSince;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The listener interface for receiving environmentalSensingDB events. The class
 * that is interested in processing a environmentalSensingDB event implements
 * this interface, and the object created with that class is registered with a
 * component using the component's
 * <code>addEnvironmentalSensingDBListener<code> method. When
 * the environmentalSensingDB event occurs, that object's appropriate
 * method is invoked.
 *
 * @author emirmos
 */
public class EnvironmentalSensingDBListener extends RPCouchDbListener {

	/** The privacy props. */
	PropertiesConfiguration privacyProps;

	/**
	 * Instantiates a new environmental sensing db listener.
	 *
	 * @param dbClient
	 *            the db client
	 * @param privacyConfig
	 *            the privacy config
	 * @param rpCloudClient
	 *            the rp cloud client
	 * @param controller
	 *            the controller
	 * @param dbName
	 *            the db name
	 * @param dbType
	 *            the db type
	 * @param since
	 *            the since
	 */
	public EnvironmentalSensingDBListener(CouchDbClient dbClient,
			LocalDataManagerConfig privacyConfig, RPCloudClient rpCloudClient,
			Controller controller, Device device,
			ContinueSince since) {
		super(dbClient, rpCloudClient, controller, device, since);
		this.logger = LoggerFactory
				.getLogger(EnvironmentalSensingDBListener.class);

		privacyProps = privacyConfig.getProperties();
	}

	/**
	 * Stop running.
	 */
	@Override
	public void stopRunning() {
		this.isRunning = false;
		logger.debug("Wait until thread EnvironmentalSensingDBListener is stopped...");
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.couchdb.RPCouchDbListener#processEvent(
	 * com.google.gson.JsonObject, java.lang.String)
	 */
	@Override
	public boolean processEvent(JsonObject feedObject, String feedSequence) {

		try {
			EnvironmentalSensing environmentalSensing = new EnvironmentalSensing();

			JsonElement timestampElement = feedObject.get("timestamp");

			if (timestampElement == null) {
				logger.warn("timestamp element not found. Ignoring document.");
				latestSeq = feedSequence;
				return true;
			}
			String timeString = timestampElement.getAsString();
			Date date = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSSX")
					.parse(timeString);
			long timestamp = date.getTime();
			environmentalSensing.setTimestamp(timestamp);

			String indoorPlaceName = device.getIndoorPlaceName();
			environmentalSensing.setIndoorPlaceName(indoorPlaceName);
			
			JsonElement element = feedObject.get("temperature");
			if (element != null
					&& privacyProps
							.getBoolean("privacy.environment.temperature")) {
				environmentalSensing.setTemperature(Double.valueOf(element.getAsDouble()));
			}

			element = feedObject.get("humidity");
			if (element != null
					&& privacyProps.getBoolean("privacy.environment.humidity")) {
				environmentalSensing.setHumidity(Double.valueOf(element.getAsDouble()));
			}

			element = feedObject.get("illuminance");
			if (element != null
					&& privacyProps
							.getBoolean("privacy.environment.illuminance")) {
				environmentalSensing.setIlluminance(Double.valueOf(element.getAsDouble()));
			}
			element = feedObject.get("movement");
			if (element != null
					&& privacyProps.getBoolean("privacy.environment.movement")) {
				environmentalSensing.setMovement(Boolean.valueOf(element.getAsBoolean()));
			}

			element = feedObject.get("CO");
			if (element != null
					&& privacyProps.getBoolean("privacy.environment.CO")) {
				environmentalSensing.setCarbonMonoxide(Integer.valueOf(element.getAsInt()));
			}

			element = feedObject.get("LPG");
			if (element != null
					&& privacyProps.getBoolean("privacy.environment.LPG")) {
				environmentalSensing.setLiquefiedPetroleumGas(Integer.valueOf(element
						.getAsInt()));
			}

			element = feedObject.get("NG");
			if (element != null
					&& privacyProps.getBoolean("privacy.environment.NG")) {
				environmentalSensing.setNaturalGas(Integer.valueOf(element.getAsInt()));
			}

			element = feedObject.get("door_open");
			if (element != null
					&& privacyProps.getBoolean("privacy.environment.door")) {
				environmentalSensing.setDoorOpen(Boolean.valueOf(element.getAsBoolean()));
			}

			boolean sendSuccessfuly = false;
			while (!sendSuccessfuly) {
				if (rpCloudClient.doPost(device.getUuid().toString(),
						DM2JsonObjectMapper
								.writeValueAsString(environmentalSensing))) {
					latestSeq = feedSequence;
					sendSuccessfuly = true;
					return true;
				}
				logger.info("Error in sending environmental data to cloud. Pausing sending for 5 minutes. ");
				this.storeLastSentUpdateSeq(latestSeq);
				logger.info("Database {} latest sent sequence stored: {}", this.getDBName(), latestSeq);
				Thread.sleep(300000);

			}

		}  catch (Exception e) {
			logger.error("{} database listener error, on sequence {}", getDBName(), feedSequence, e);
		} 

		latestSeq = feedSequence;
		return true;

	}
}
