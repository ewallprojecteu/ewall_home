package eu.ewall.platform.local.datamanager.couchdb;

import java.text.SimpleDateFormat;
import java.util.ArrayList;
import java.util.Date;
import java.util.List;

import org.apache.commons.configuration.PropertiesConfiguration;
import org.lightcouch.CouchDbClient;
import org.slf4j.LoggerFactory;

import com.google.gson.JsonArray;
import com.google.gson.JsonElement;
import com.google.gson.JsonObject;

import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.commons.datamodel.marshalling.json.DM2JsonObjectMapper;
import eu.ewall.platform.commons.datamodel.profile.EmotionalStateCategory;
import eu.ewall.platform.commons.datamodel.profile.VCardGenderType;
import eu.ewall.platform.commons.datamodel.sensing.VisualSensing;
import eu.ewall.platform.local.datamanager.config.LocalDataManagerConfig;
import eu.ewall.platform.local.datamanager.controller.Controller;
import eu.ewall.platform.remoteproxy.api.ContinueSince;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The listener interface for receiving visualSensingDB events. The class that
 * is interested in processing a visualSensingDB event implements this
 * interface, and the object created with that class is registered with a
 * component using the component's
 * <code>addVisualSensingDBListener<code> method. When
 * the visualSensingDB event occurs, that object's appropriate
 * method is invoked.
 *
 * @author emirmos
 */
public class VisualSensingDBListener extends RPCouchDbListener {

	/** The privacy props. */
	PropertiesConfiguration privacyProps;

	/**
	 * Instantiates a new visual sensing db listener.
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
	public VisualSensingDBListener(CouchDbClient dbClient,
			LocalDataManagerConfig privacyConfig, RPCloudClient rpCloudClient,
			Controller controller,  Device device,
			ContinueSince since) {
		super(dbClient, rpCloudClient, controller, device, since);
		this.logger = LoggerFactory.getLogger(VisualSensingDBListener.class);
		privacyProps = privacyConfig.getProperties();
	}

	/**
	 * Stop running.
	 */
	@Override
	public void stopRunning() {
		this.isRunning = false;
		logger.debug("Wait until thread VisualSensingDBListener is stopped...");
	}

	/**
	 * Process event.
	 *
	 * @param feedObject
	 *            the feed object
	 * @param feedSequence
	 *            the feed sequence
	 * @return true, if successful
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

			JsonElement peopleElement = feedObject.get("people");
			if (peopleElement != null
					&& privacyProps.getBoolean("privacy.visual")) {

				JsonArray peopleArray = peopleElement.getAsJsonArray();
				List<VisualSensing> vsList = new ArrayList<VisualSensing>();
				for (JsonElement personElement : peopleArray) {
					VisualSensing vs = new VisualSensing();

					JsonObject personObject = personElement.getAsJsonObject();
					JsonElement element = personObject.get("trackID");
					if (element != null) {
						vs.setTrack_id(element.getAsInt());
					}

					element = personObject.get("x");
					if (element != null) {
						vs.setX(element.getAsInt());
					}

					element = personObject.get("y");
					if (element != null) {
						vs.setY(element.getAsInt());
					}

					element = personObject.get("width");
					if (element != null) {
						vs.setWidth(element.getAsInt());
					}

					element = personObject.get("height");
					if (element != null) {
						vs.setHeight(element.getAsInt());
					}

					element = personObject.get("positionConf");
					if (element != null) {
						vs.setPositionConf(element.getAsDouble());
					}

					element = personObject.get("gender");
					if (element != null) {
						vs.setGender(VCardGenderType.valueOf(element
								.getAsString()));
					}

					element = personObject.get("genderConf");
					if (element != null) {
						vs.setGenderConf(element.getAsDouble());
					}

					element = personObject.get("age");
					if (element != null) {
						vs.setAge(element.getAsInt());
					}

					element = personObject.get("ageConf");
					if (element != null) {
						vs.setAgeConf(element.getAsDouble());
					}

					element = personObject.get("emotion");
					if (element != null) {
						vs.setEmotion(EmotionalStateCategory.valueOf(element
								.getAsString()));
					}

					element = personObject.get("emotionConf");
					if (element != null) {
						vs.setEmotionConf(element.getAsDouble());
					}

					vs.setTimestamp(timestamp);

					vsList.add(vs);
				}

				boolean sendSuccessfuly = false;
				while (!sendSuccessfuly) {
					boolean errorOccured = false;
					for (VisualSensing vsData : vsList) {
						String vsDataString = DM2JsonObjectMapper
								.writeValueAsString(vsData);
						if (!rpCloudClient.doPostVisual(
								device.getUuid().toString(),
								vsDataString)) {
							errorOccured = true;
						}
					}

					if (!errorOccured) {
						latestSeq = feedSequence;
						sendSuccessfuly = true;
						return true;
					}
					logger.info("Error in sending visual data to cloud. Pausing sending for 5 minutes. ");
					this.storeLastSentUpdateSeq(latestSeq);
					logger.info("Database {} latest sent sequence stored: {}", this.getDBName(), latestSeq);
					Thread.sleep(300000);

				}

			}
		}  catch (Exception e) {
			logger.error("{} database listener error, on sequence {}", getDBName(), feedSequence, e);
		} 

		latestSeq = feedSequence;
		return true;
	}
}
