package eu.ewall.platform.local.datamanager.couchdb;

import java.text.ParseException;
import java.text.SimpleDateFormat;
import java.util.Date;

import org.apache.commons.configuration.PropertiesConfiguration;
import org.lightcouch.CouchDbClient;
import org.slf4j.LoggerFactory;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import eu.ewall.platform.commons.datamodel.activity.ActivityType;
import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.commons.datamodel.marshalling.json.DM2JsonObjectMapper;
import eu.ewall.platform.commons.datamodel.measure.AccelerometerMeasurement;
import eu.ewall.platform.local.datamanager.config.LocalDataManagerConfig;
import eu.ewall.platform.local.datamanager.controller.Controller;
import eu.ewall.platform.remoteproxy.api.ContinueSince;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The Class RPActivityDbClient.
 *
 * @author emirmos
 */
public class ActivitySensingDBListener extends RPCouchDbListener {

	/** The privacy props. */
	PropertiesConfiguration privacyProps;

	/**
	 * Instantiates a new activity sensing db listener.
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
	public ActivitySensingDBListener(CouchDbClient dbClient,
			LocalDataManagerConfig privacyConfig, RPCloudClient rpCloudClient,
			Controller controller, Device device,
			ContinueSince since) {
		super(dbClient, rpCloudClient, controller, device, since);
		this.logger = LoggerFactory.getLogger(ActivitySensingDBListener.class);
		privacyProps = privacyConfig.getProperties();
	}

	/**
	 * Stop running.
	 */
	@Override
	public void stopRunning() {
		this.isRunning = false;
		logger.debug("Wait until thread ActivitySensingDBListener is stopped...");
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
				logger.warn("Timestamp element not found. Ignoring document.");
				latestSeq = feedSequence;
				return true;

			}
			String timeString = timestampElement.getAsString();
			Date date = new SimpleDateFormat("yyyy-MM-dd'T'HH:mm:ss.SSSX")
					.parse(timeString);
			long timestamp = date.getTime();

			JsonElement element = feedObject.get("activity");
			JsonElement fallElement = feedObject.get("fall");

			AccelerometerMeasurement accMeasurement = new AccelerometerMeasurement();

			if (element != null && privacyProps.getBoolean("privacy.activity")) {
				JsonObject jsonObject = element.getAsJsonObject();
				JsonElement imaElement = jsonObject.get("IMA");
				JsonElement isaElement = jsonObject.get("ISA");
				JsonElement stepsElement = jsonObject.get("steps");
				JsonElement physicalActivityElement = jsonObject
						.get("physicalActivity");

				if (imaElement != null)
					accMeasurement.setImaValue(imaElement.getAsDouble());
				if (isaElement != null)
					accMeasurement.setIsaValue(isaElement.getAsDouble());
				if (stepsElement != null)
					accMeasurement.setSteps(stepsElement.getAsLong());
				if (physicalActivityElement != null)
					accMeasurement.setActivityType(ActivityType
							.valueOf(physicalActivityElement.getAsString()));

			}
			if (fallElement != null && privacyProps.getBoolean("privacy.fall")) {
				accMeasurement.setFallDetected(fallElement.getAsBoolean());
			}

			if ((element != null && privacyProps.getBoolean("privacy.activity"))
					|| (fallElement != null && privacyProps
							.getBoolean("privacy.fall"))) {
				accMeasurement.setTimestamp(timestamp);
				String activiyString = DM2JsonObjectMapper
						.writeValueAsString(accMeasurement);

				if (activiyString != null) {
					boolean sendSuccessfuly = false;
					while (!sendSuccessfuly) {
						if (rpCloudClient.doPostAcc(
								device.getUuid().toString(),
								activiyString)) {
							latestSeq = feedSequence;
							sendSuccessfuly = true;
							return true;
						}
						logger.info("Error in sending activity data to cloud. Pausing sending for 5 minutes. ");
						this.storeLastSentUpdateSeq(latestSeq);
						logger.info("Database {} latest sent sequence stored: {}", this.getDBName(), latestSeq);
						Thread.sleep(300000);
					}

				}
			}
		}  catch (Exception e) {
			logger.error("{} database listener error, on sequence {}", getDBName(), feedSequence, e);
		} 

		latestSeq = feedSequence;
		return true;

	}
}
