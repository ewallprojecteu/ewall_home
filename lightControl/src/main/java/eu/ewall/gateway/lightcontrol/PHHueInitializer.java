package eu.ewall.gateway.lightcontrol;

import java.util.List;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.philips.lighting.hue.sdk.PHAccessPoint;
import com.philips.lighting.hue.sdk.PHHueSDK;
import com.philips.lighting.hue.sdk.PHMessageType;
import com.philips.lighting.hue.sdk.PHSDKListener;
import com.philips.lighting.model.PHBridge;

import eu.ewall.gateway.lightcontrol.config.LightSystemProperties;

/**
 * The Class PHHueInitializer.
 */
public class PHHueInitializer {

	/** The log. */
	private static Logger LOG = LoggerFactory.getLogger(PHHueInitializer.class);

	/**
	 * Initialize.
	 *
	 * @throws Exception
	 *             the exception
	 */
	public static void initialize() throws Exception {

		final PHHueSDK phHueSDK = PHHueSDK.create();

		phHueSDK.setAppName("lightControl");
		phHueSDK.setDeviceName("eWall");

		LightSystemProperties.loadProperties();

		PHSDKListener listener = new PHSDKListener() {

			@Override
			public void onAccessPointsFound(List accessPoint) {
				LOG.info("Access point found: " + accessPoint);
				phHueSDK.getAccessPointsFound().addAll(accessPoint);
				phHueSDK.connect(phHueSDK.getAccessPointsFound().get(0));
			}

			@Override
			public void onCacheUpdated(List cacheNotificationsList,
					PHBridge bridge) {
				// Here you receive notifications that the BridgeResource Cache
				// was updated. Use the PHMessageType to
				// check which cache was updated, e.g.
				if (cacheNotificationsList
						.contains(PHMessageType.LIGHTS_CACHE_UPDATED)) {
					LOG.info("Lights Cache Updated ");
				}
			}

			@Override
			public void onBridgeConnected(PHBridge b, String username) {
				LOG.info("Bridge connected with username: " + username);
				phHueSDK.setSelectedBridge(b);
				phHueSDK.enableHeartbeat(b, PHHueSDK.HB_INTERVAL);
				String lastIpAddress = b.getResourceCache()
						.getBridgeConfiguration().getIpAddress();
				LightSystemProperties.storeUsername(username);
				LightSystemProperties.storeLastIPAddress(lastIpAddress);
				LightSystemProperties.saveProperties();

				// Here it is recommended to set your connected bridge in your
				// sdk object (as above) and start the heartbeat.
				// At this point you are connected to a bridge so you should
				// pass control to your main program/activity.
				// The username is generated randomly by the bridge.
				// Also it is recommended you store the connected IP Address/
				// Username in your app here. This will allow easy automatic
				// connection on subsequent use.
			}

			@Override
			public void onAuthenticationRequired(PHAccessPoint accessPoint) {
				LOG.info("If you are installing the Philips Hue for the first time, please push link button now for authentication (located on the top of your bridge device) - waiting for 30 seconds");

				phHueSDK.startPushlinkAuthentication(accessPoint);

				// Arriving here indicates that Pushlinking is required (to
				// prove the User has physical access to the bridge). Typically
				// here
				// you will display a pushlink image (with a timer) indicating
				// to to the user they need to push the button on their bridge
				// within 30 seconds.
			}

			@Override
			public void onConnectionResumed(PHBridge bridge) {

			}

			@Override
			public void onConnectionLost(PHAccessPoint accessPoint) {
				LOG.info("Bridge connection lost ");
				try {
					PHHueInitializer.initialize();
				} catch (Exception e) {
					LOG.warn(e.getMessage());
				}
			}

			@Override
			public void onError(int code, final String message) {
//				LOG.info("Bridge not found or not responding yet ");

			}

			@Override
			public void onParsingErrors(List parsingErrorsList) {
				LOG.warn("JSON parsing error ");
			}
		};

		phHueSDK.getNotificationManager().registerSDKListener(listener);

		PHHueController controller = PHHueController.getInstance();
		/*
		 * findBridges done in connectToLastKnownAccessPoint method because of
		 * performance boost
		 */
		// controller.findBridges();
		controller.connectToLastKnownAccessPoint();
	}

}
