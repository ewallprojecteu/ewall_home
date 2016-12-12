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
import eu.ewall.platform.commons.datamodel.measure.MattressPressureSensing;
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
public class FurnitureSensingDBListener extends RPCouchDbListener {

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
	public FurnitureSensingDBListener(CouchDbClient dbClient,
			LocalDataManagerConfig privacyConfig, RPCloudClient rpCloudClient,
			Controller controller,  Device device,
			ContinueSince since) {
		super(dbClient, rpCloudClient, controller, device, since);
		this.logger = LoggerFactory.getLogger(FurnitureSensingDBListener.class);
		privacyProps = privacyConfig.getProperties();
	}

	/**
	 * Stop running.
	 */
	@Override
	public void stopRunning() {
		this.isRunning = false;
		logger.debug("Wait until thread FurnitureSensingDBListener is stopped...");
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

			JsonElement element = feedObject.get("pressure");
			if (element != null) {
				boolean pressure = element.getAsBoolean();
				element = feedObject.get("IMA");
				double ima = 0;
				if (element != null) {
					ima = element.getAsDouble();
				}
				MattressPressureSensing pressureSensing = new MattressPressureSensing(
						pressure, ima, timestamp);

				pressureSensing.setIndoorPlaceName(device.getIndoorPlaceName());

				boolean sendSuccessfuly = false;

				// only sends the data to the cloud if the privacy setting for
				// pressure sensor in that location is set to true
				if (privacyProps.getBoolean("privacy.pressure")) {
					while (!sendSuccessfuly) {
						if (rpCloudClient.doPostPress(
								device.getUuid().toString(),
								DM2JsonObjectMapper
										.writeValueAsString(pressureSensing))) {
							latestSeq = feedSequence;
							sendSuccessfuly = true;
							return true;
						}
						logger.info("Error in sending furniture pressure data to cloud. Pausing sending for 5 minutes. ");
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
