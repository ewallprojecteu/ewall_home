package eu.ewall.platform.local.datamanager.couchdb;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.commons.configuration.PropertiesConfiguration;
import org.lightcouch.CouchDbClient;
import org.slf4j.LoggerFactory;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.commons.datamodel.marshalling.json.DM2JsonObjectMapper;
import eu.ewall.platform.commons.datamodel.sensing.AppliancePowerSensing;
import eu.ewall.platform.local.datamanager.config.LocalDataManagerConfig;
import eu.ewall.platform.local.datamanager.controller.Controller;
import eu.ewall.platform.remoteproxy.api.ContinueSince;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The listener interface for receiving appliancePowerSensingDB events. The
 * class that is interested in processing a appliancePowerSensingDB event
 * implements this interface, and the object created with that class is
 * registered with a component using the component's
 * <code>addAppliancePowerSensingDBListener<code> method. When
 * the appliancePowerSensingDB event occurs, that object's appropriate
 * method is invoked.
 *
 * @author emirmos
 */
public class AppliancePowerSensingDBListener extends RPCouchDbListener {

	/** The privacy props. */
	PropertiesConfiguration privacyProps;

	/**
	 * Instantiates a new appliance power sensing db listener.
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
	public AppliancePowerSensingDBListener(CouchDbClient dbClient,
			LocalDataManagerConfig privacyConfig, RPCloudClient rpCloudClient,
			Controller controller,  Device device,
			ContinueSince since) {
		super(dbClient, rpCloudClient, controller,  device, since);
		this.logger = LoggerFactory
				.getLogger(AppliancePowerSensingDBListener.class);
		privacyProps = privacyConfig.getProperties();
	}

	/**
	 * Stop running.
	 */
	@Override
	public void stopRunning() {
		this.isRunning = false;
		logger.debug("Wait until thread AppliancePowerSensingDBListener is stopped...");
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

			JsonElement element = feedObject.get("appliances");
			AppliancePowerSensing aps = new AppliancePowerSensing();

			if (element != null
					&& privacyProps.getBoolean("privacy.appliances")) {

				JsonArray jsonArray = element.getAsJsonArray();

				for (JsonElement applianceElement : jsonArray) {
					JsonObject jsonObject = applianceElement.getAsJsonObject();
					JsonElement name = jsonObject.get("name");

					if (name != null)
						aps.setApplianceName(name.getAsString());
					else {
						logger.warn("Name of appliance is not defined. Ignoring document with timestamp "
								+ timeString);
						latestSeq = feedSequence;
						return true;
					}

					JsonElement roomName = jsonObject.get("room");
					JsonElement socketOn = jsonObject.get("socketOn");
					JsonElement powerNow = jsonObject.get("powerNow");
					JsonElement dailyEnergy = jsonObject.get("dailyEnergy");
					if (roomName != null)
						aps.setIndoorPlaceName(roomName.getAsString());
					// TODO do we need to check if this room is defined in
					// sensing environment configuration?
					if (socketOn != null)
						aps.setSocketOn(socketOn.getAsBoolean());

					if (powerNow != null)
						aps.setPowerNow(powerNow.getAsDouble());

					if (dailyEnergy != null)
						aps.setDailyEnergy(dailyEnergy.getAsDouble());

					aps.setTimestamp(timestamp);

					boolean sendSuccessfuly = false;
					while (!sendSuccessfuly) {
						if (rpCloudClient.doPostAppliancePower(
								device.getUuid().toString(),
								DM2JsonObjectMapper.writeValueAsString(aps))) {
							sendSuccessfuly = true;
						} else {
							logger.info("Error in sending appliance power sensing data to cloud. Pausing sending for 5 minutes. ");
							this.storeLastSentUpdateSeq(latestSeq);
							logger.info("Database {} latest sent sequence stored: {}", this.getDBName(), latestSeq);
							Thread.sleep(300000);
						}

					}
				}
			}

		} catch (Exception e) {
			logger.error("{} database listener error, on sequence {}", getDBName(), feedSequence, e);
		} 
		
		latestSeq = feedSequence;
		return true;

	}
}
