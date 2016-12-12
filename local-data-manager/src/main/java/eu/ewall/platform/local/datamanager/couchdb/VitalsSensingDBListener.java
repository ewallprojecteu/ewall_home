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
import eu.ewall.platform.commons.datamodel.sensing.VitalsSensing;
import eu.ewall.platform.local.datamanager.config.LocalDataManagerConfig;
import eu.ewall.platform.local.datamanager.controller.Controller;
import eu.ewall.platform.remoteproxy.api.ContinueSince;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The listener interface for receiving healthSensingDB events. The class that
 * is interested in processing a healthSensingDB event implements this
 * interface, and the object created with that class is registered with a
 * component using the component's
 * <code>addHealthSensingDBListener<code> method. When
 * the healthSensingDB event occurs, that object's appropriate
 * method is invoked.
 *
 * @author emirmos
 */
public class VitalsSensingDBListener extends RPCouchDbListener {

	/** The privacy props. */
	PropertiesConfiguration privacyProps;

	/**
	 * Instantiates a new health sensing db listener.
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
	public VitalsSensingDBListener(CouchDbClient dbClient,
			LocalDataManagerConfig privacyConfig, RPCloudClient rpCloudClient,
			Controller controller,  Device device,
			ContinueSince since) {
		super(dbClient, rpCloudClient, controller, device, since);
		this.logger = LoggerFactory.getLogger(VitalsSensingDBListener.class);
		privacyProps = privacyConfig.getProperties();
	}

	/**
	 * Stop running.
	 */
	@Override
	public void stopRunning() {
		this.isRunning = false;
		logger.debug("Wait until thread HealthSensingDBListener is stopped...");
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

			JsonElement element = feedObject.get("SPO2");
			VitalsSensing vs = new VitalsSensing();
			if (element != null
					&& privacyProps.getBoolean("privacy.vitals.SPO2")) {
				vs.setOxygenSaturation(element.getAsInt());
			} else
				vs.setOxygenSaturation(-1);

			element = feedObject.get("HR");
			if (element != null && privacyProps.getBoolean("privacy.vitals.HR")) {
				vs.setHearthRate(element.getAsInt());
			} else
				vs.setHearthRate(-1);

			element = feedObject.get("HRV");
			if (element != null
					&& privacyProps.getBoolean("privacy.vitals.HRV")) {
				vs.setHearthRateVariability(element.getAsInt());
			} else
				vs.setHearthRateVariability(-1);

			element = feedObject.get("systolicBP");
			if (element != null
					&& privacyProps.getBoolean("privacy.vitals.systolicBP")) {
				vs.setSystolicBloodPressure(element.getAsInt());
			} else
				vs.setSystolicBloodPressure(-1);

			element = feedObject.get("diastolicBP");
			if (element != null
					&& privacyProps.getBoolean("privacy.vitals.diastolicBP")) {
				vs.setDiastolicBloodPressure(element.getAsInt());
			} else
				vs.setDiastolicBloodPressure(-1);

			vs.setTimestamp(timestamp);

			controller.processVitalsSensingData(vs);

			boolean sendSuccessfuly = false;
			while (!sendSuccessfuly) {
				if (rpCloudClient.doPostVitals(
						device.getUuid().toString(),
						DM2JsonObjectMapper.writeValueAsString(vs))) {
					latestSeq = feedSequence;
					sendSuccessfuly = true;
					return true;
				}
				logger.info("Error in sending vitals data to cloud. Pausing sending for 5 minutes.");
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
