package eu.ewall.gateway.lightcontrol;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.philips.lighting.hue.sdk.PHHueSDK;

/**
 * The Class PHHueKiller.
 */
public class PHHueKiller {

	/** The log. */
	private static Logger LOG = LoggerFactory.getLogger(PHHueKiller.class);

	/**
	 * Kill.
	 */
	public static void kill() {

		PHHueSDK phHueSDK = PHHueSDK.create();
		PHHueController controller = PHHueController.getInstance();
		if (phHueSDK.getSelectedBridge() == null){
			LOG.info("Stopping bridge search due to timeout ");
		}
		else{
		LOG.info("Disconnecting from the bridge");
		}
		phHueSDK.disableAllHeartbeat();
		phHueSDK.disconnect(phHueSDK.getSelectedBridge());
		controller.disconnect();
		LOG.info("Disconnect completed");

	}

}
