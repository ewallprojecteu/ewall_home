package eu.ewall.gateway.lightcontrol;

import java.util.List;
import java.util.Random;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.philips.lighting.hue.sdk.PHAccessPoint;
import com.philips.lighting.hue.sdk.PHBridgeSearchManager;
import com.philips.lighting.hue.sdk.PHHueSDK;
import com.philips.lighting.hue.sdk.utilities.PHUtilities;
import com.philips.lighting.hue.sdk.utilities.impl.Color;
import com.philips.lighting.model.PHBridge;
import com.philips.lighting.model.PHBridgeResourcesCache;
import com.philips.lighting.model.PHLight;
import com.philips.lighting.model.PHLightState;

import eu.ewall.gateway.lightcontrol.config.LightSystemProperties;

/**
 * The Class PHHueController.
 */
public class PHHueController {

	/** The log. */
	private static Logger LOG = LoggerFactory.getLogger(PHHueController.class);

	/** The ph hue sdk. */
	private PHHueSDK phHueSDK = PHHueSDK.create();

	/** The Constant MAX_HUE. */
	private static final int MAX_HUE = 65535;

	/** The Constant MAX_INTENSITY. */
	private static final int MAX_INTENSITY = 254;

	/** The instance. */
	private static PHHueController instance;

	/**
	 * Instantiates a new PH hue controller.
	 */
	private PHHueController() {
	}

	/**
	 * Gets the single instance of PHHueController.
	 *
	 * @return single instance of PHHueController
	 */
	public static PHHueController getInstance() {
		if (instance == null) {
			instance = new PHHueController();
		}
		return instance;
	}

	/**
	 * Find bridges.
	 */
	public void findBridges() {
		phHueSDK = PHHueSDK.getInstance();
		PHBridgeSearchManager sm = (PHBridgeSearchManager) phHueSDK
				.getSDKService(PHHueSDK.SEARCH_BRIDGE);
		sm.search(true, true);
		LOG.info("Bridge search started");
		try {
			Thread.sleep(10000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

	}

	/**
	 * Random lights.
	 */
	public void randomLights() {
		PHBridge bridge = phHueSDK.getSelectedBridge();
		PHBridgeResourcesCache cache = bridge.getResourceCache();

		List<PHLight> allLights = cache.getAllLights();
		Random rand = new Random();

		for (PHLight light : allLights) {
			PHLightState lightState = new PHLightState();
			lightState.setHue(rand.nextInt(MAX_HUE));
			bridge.updateLightState(light, lightState); // If no bridge response
														// is required then use
														// this simpler form.
		}
	}

	/**
	 * Sets the light.<br>
	 * Function implemented for library independent calls from outside classes
	 *
	 * @param light
	 *            the light
	 * @param hsv
	 *            the hsv
	 */
	public void setLight(int light, float[] hsv) {
		if (checkIfLightExists(light)) {
			setLight(phHueSDK.getSelectedBridge().getResourceCache()
					.getAllLights().get(light), hsv);
		} else {
			LOG.warn("You tried selecting a light that doesn't exist: Light "
					+ light);
			LOG.warn("There are only "
					+ phHueSDK.getSelectedBridge().getResourceCache()
							.getAllLights().size() + " lights available");
		}
	}

	/**
	 * Sets the light.
	 *
	 * @param light
	 *            the light
	 * @param hsv
	 *            the hsv
	 */
	public void setLight(PHLight light, float[] hsv) {
		PHBridge bridge = phHueSDK.getSelectedBridge();
		int hue = Math.round(hsv[0] * 65535);
		PHLightState lightState = new PHLightState();
		lightState.setHue(hue);
		bridge.updateLightState(light, lightState); // Set a desired light to
													// desired color
	}
	
	public void setLightRGB(PHLight light, int rgbInt) {
		int red = Color.red(rgbInt);
		int green = Color.green(rgbInt);
		int blue = Color.blue(rgbInt);

		float xy[] = PHUtilities.calculateXYFromRGB(red, green, blue, light.getModelNumber());
		PHLightState lightState = new PHLightState();
		lightState.setX(xy[0]);
		lightState.setY(xy[1]);
		PHBridge bridge = phHueSDK.getSelectedBridge();
		bridge.updateLightState(light, lightState); 
		
	}
	
	public void setLightRGBAll(int rgbInt) {
		List<PHLight> lights = phHueSDK.getSelectedBridge().getResourceCache()
				.getAllLights();
		for (PHLight light : lights) {
			setLightRGB(light, rgbInt);
		}
		
	}

	/**
	 * Sets the light.<br>
	 * Function implemented for library independent calls from outside classes
	 *
	 * @param light
	 *            the light
	 * @param hue
	 *            the hue
	 */
	public void setLight(int light, int hue) {
		if (checkIfLightExists(light)) {
			setLight(phHueSDK.getSelectedBridge().getResourceCache()
					.getAllLights().get(light), hue);
		} else {
			LOG.warn("You tried selecting a light that doesn't exist: Light "
					+ light);
			LOG.warn("There are only "
					+ phHueSDK.getSelectedBridge().getResourceCache()
							.getAllLights().size() + " lights available");
		}
	}

	/**
	 * Sets the light.
	 *
	 * @param light
	 *            the light
	 * @param hue
	 *            the hue
	 */
	public void setLight(PHLight light, int hue) {
		PHBridge bridge = phHueSDK.getSelectedBridge();

		PHLightState lightState = new PHLightState();
		lightState.setHue(hue);
		bridge.updateLightState(light, lightState); // Set a desired light to
													// desired color
	}

	/**
	 * Sets the light intensity increment.<br>
	 * Function implemented for library independent calls from outside classes
	 *
	 * @param light
	 *            the light
	 * @param intensityLevel
	 *            the intensity level
	 */
	public void setLightIntensityIncrement(int light, int intensityLevel) {
		if (checkIfLightExists(light)) {
			changeLightIntensityIncrement(phHueSDK.getSelectedBridge()
					.getResourceCache().getAllLights().get(light),
					intensityLevel);
		} else {
			LOG.warn("You tried selecting a light that doesn't exist: Light "
					+ light);
			LOG.warn("There are only "
					+ phHueSDK.getSelectedBridge().getResourceCache()
							.getAllLights().size() + " lights available");
		}
	}

	/**
	 * Sets the light intensity.
	 *
	 * @param light
	 *            the light
	 * @param intensityLevel
	 *            the intensity level
	 */
	public void setLightIntensity(int light, int intensityLevel) {
		if (checkIfLightExists(light)) {
			setLightIntensity(phHueSDK.getSelectedBridge().getResourceCache()
					.getAllLights().get(light), intensityLevel);
		} else {
			LOG.warn("You tried selecting a light that doesn't exist: Light "
					+ light);
			LOG.warn("There are only "
					+ phHueSDK.getSelectedBridge().getResourceCache()
							.getAllLights().size() + " lights available");
		}
	}

	/**
	 * Sets the light intensity.
	 *
	 * @param light
	 *            the light
	 * @param intensityLevel
	 *            the intensity level
	 */
	public void setLightIntensity(PHLight light, int intensityLevel) {
		if (intensityLevel < 0 || intensityLevel > 254) {
			throw new IllegalArgumentException();
		}
		PHBridge bridge = phHueSDK.getSelectedBridge();

		PHLightState lightState = new PHLightState();
		lightState.setBrightness(intensityLevel);
		bridge.updateLightState(light, lightState); // Set a desired light to
													// desired brightness level
	}

	/**
	 * Sets the light intensity percent.
	 *
	 * @param light
	 *            the light
	 * @param intensityLevel
	 *            the intensity level
	 */
	public void setLightIntensityPercent(int light, int intensityLevel) {
		if (light == -1) {
			//no specific light was selected, will be applied to all connected lights (as agreed with wp5)
			this.setAllLightsIntensityPercent(intensityLevel);
			return;
		}
		if (checkIfLightExists(light)) {
			setLightIntensityPercent(getLightWithID(light),
					intensityLevel);
		} else {
			LOG.warn("You tried selecting a light that doesn't exist: Light "
					+ light);
			LOG.warn("There are only "
					+ phHueSDK.getSelectedBridge().getResourceCache()
							.getAllLights().size() + " lights available");
		}
	}
	

	public void setAllLightsIntensityPercent(int value) {
		List<PHLight> lights = phHueSDK.getSelectedBridge().getResourceCache()
				.getAllLights();
		for (PHLight light : lights) {
			setLightIntensityPercent(light, value);
		}

	}

	/**
	 * Sets the light intensity percent.
	 *
	 * @param light
	 *            the light
	 * @param percentage
	 *            the percentage
	 */
	public void setLightIntensityPercent(PHLight light, int percentage) {
		if (percentage < 0 || percentage > 100) {
			throw new IllegalArgumentException();
		}
		PHBridge bridge = phHueSDK.getSelectedBridge();

		double percentageDouble = percentage;
		percentageDouble = (percentageDouble / 100);
		int intensityLevel = (int) (MAX_INTENSITY * percentageDouble);

		PHLightState lightState = new PHLightState();
		lightState.setBrightness(intensityLevel);
		bridge.updateLightState(light, lightState); // Set a desired light to
													// desired brightness level
	}

	/**
	 * Change light intensity increment.<br>
	 * Function implemented for library independent calls from outside classes
	 *
	 * @param light
	 *            the light
	 * @param value
	 *            the value
	 */
	public void changeLightIntensityIncrement(int light, int value) {
		if (checkIfLightExists(light)) {
			changeLightIntensityIncrement(phHueSDK.getSelectedBridge()
					.getResourceCache().getAllLights().get(light), value);
		} else {
			LOG.warn("You tried selecting a light that doesn't exist: Light "
					+ light);
			LOG.warn("There are only "
					+ phHueSDK.getSelectedBridge().getResourceCache()
							.getAllLights().size() + " lights available");
		}
	}

	/**
	 * Change light intensity percent.
	 *
	 * @param light
	 *            the light
	 * @param value
	 *            the value
	 */
	public void changeLightIntensityPercent(int light, int value) {
		if (light == -1) {
			//no specific light was selected, will be applied to all connected lights (as agreed with wp5)
			this.changeAllLightsIntensityPercent(value);
			return;
		}
		if (checkIfLightExists(light)) {
			changeLightIntensityPercent(getLightWithID(light), value);
		} else {
			LOG.warn("You tried selecting a light that doesn't exist: Light "
					+ light);
			LOG.warn("There are only "
					+ phHueSDK.getSelectedBridge().getResourceCache()
							.getAllLights().size() + " lights available");
		}
	}

	public void changeAllLightsIntensityPercent(int value) {
		List<PHLight> lights = phHueSDK.getSelectedBridge().getResourceCache()
				.getAllLights();

		for (PHLight light : lights) {
			changeLightIntensityPercent(light, value);
		}

	}
	
	/**
	 * Change light intensity increment.
	 *
	 * @param light
	 *            the light
	 * @param value
	 *            the value
	 */
	public void changeLightIntensityIncrement(PHLight light, int value) {
		if (value < -254 || value > 254) {
			throw new IllegalArgumentException();
		}
		PHBridge bridge = phHueSDK.getSelectedBridge();
		PHLightState state = light.getLastKnownLightState();
		int intensityLevel = state.getBrightness();

		if (intensityLevel + value > 254) {
			intensityLevel = 254;
		} else if (intensityLevel + value < 0) {
			intensityLevel = 0;
		} else {
			intensityLevel = intensityLevel + value;
		}
		PHLightState lightState = new PHLightState();
		lightState.setBrightness(intensityLevel);
		bridge.updateLightState(light, lightState); // Change brightness by a
													// fixed increment
	}

	/**
	 * Change light intensity percent in regard to MAX_INTENSITY.
	 *
	 * @param light
	 *            the light
	 * @param percentage
	 *            the percentage
	 */
	public void changeLightIntensityPercent(PHLight light, int percentage) {
		if (percentage < -100 || percentage > 100) {
			throw new IllegalArgumentException();
		}
		PHBridge bridge = phHueSDK.getSelectedBridge();
		PHLightState state = light.getLastKnownLightState();
		int intensityLevel = state.getBrightness();
		double percentageDouble = percentage;

		if (percentageDouble >= 0) {
			percentageDouble = (percentageDouble / 100);
			intensityLevel = (int) (intensityLevel + (MAX_INTENSITY * percentageDouble));
			if (intensityLevel > MAX_INTENSITY) {
				intensityLevel = MAX_INTENSITY;
			}
		} else {
			percentageDouble = Math.abs((percentageDouble / 100));
			intensityLevel = (int) (intensityLevel - (MAX_INTENSITY * percentageDouble));
			if (intensityLevel < 0) {
				intensityLevel = 0;
			}
		}

		PHLightState lightState = new PHLightState();
		lightState.setBrightness(intensityLevel);
		bridge.updateLightState(light, lightState); // Change brightness as a
													// percentage of current
													// brightness
	}

	/**
	 * Check if light exists.<br>
	 * Function implemented for library independent calls from outside classes
	 *
	 * @param light
	 *            the light
	 * @return true, if successful
	 */
	public boolean checkIfLightExists(int light) {

		if (light >= phHueSDK.getSelectedBridge().getResourceCache()
				.getAllLights().size()) {
			return false;
		} else {
			return true;
		}
	}

	/**
	 * On off.<br>
	 * Function implemented for library independent calls from outside classes
	 *
	 * @param light
	 *            the light
	 * @param state
	 *            the state
	 */
	public void onOff(int light, boolean state) {
		if (light == -1) {
			//no specific light was selected, will be applied to all connected lights (as agreed with wp5)
			onOffAll(state);
			return;
		}
		if (checkIfLightExists(light)) {
			onOff(getLightWithID(light), state);
		} else {
			LOG.warn("You tried selecting a light that doesn't exist: Light "
					+ light);
			LOG.warn("There are only "
					+ phHueSDK.getSelectedBridge().getResourceCache()
							.getAllLights().size() + " lights available");
		}
	}
	
	public void onOffAll(boolean state) {
		List<PHLight> lights = phHueSDK.getSelectedBridge().getResourceCache()
				.getAllLights();
		for (PHLight light : lights) {
			onOff(light, state);
		}

	}

	private PHLight getLightWithID(int lightID) {
		List<PHLight> lights = phHueSDK.getSelectedBridge().getResourceCache()
		.getAllLights();
		
		for (PHLight light: lights) {
			if (light.getIdentifier().equals(String.valueOf(lightID)))
				return light;
		}		
		return null;
	}

	/**
	 * On off.
	 *
	 * @param light
	 *            the light
	 * @param state
	 *            the state
	 */
	public void onOff(PHLight light, boolean state) {
		PHBridge bridge = phHueSDK.getSelectedBridge();
		PHLightState lightState = new PHLightState();
		if (state == false) {
			lightState.setOn(false);
		} else {
			lightState.setOn(true);

		}
		bridge.updateLightState(light, lightState); // Turns on or off the
													// selected light
	}

	/**
	 * Rgb to hsb.
	 *
	 * @param r
	 *            the r
	 * @param g
	 *            the g
	 * @param b
	 *            the b
	 * @return the float[]
	 */
	public static float[] rgbToHsb(int r, int g, int b) {
		float[] hsv = java.awt.Color.RGBtoHSB(r, g, b, null);

		return hsv;
	}

	/**
	 * Connect to the last known access point. This method is triggered by the
	 * Connect to Bridge button but it can equally be used to automatically
	 * connect to a bridge.
	 *
	 * @return true, if successful
	 */
	public boolean connectToLastKnownAccessPoint() {
		String username = LightSystemProperties.getUsername();
		String lastIpAddress = LightSystemProperties.getLastConnectedIP();

		if (username == null || lastIpAddress == null) {
			return false;
		}
		PHAccessPoint accessPoint = new PHAccessPoint();
		accessPoint.setIpAddress(lastIpAddress);
		accessPoint.setUsername(username);

		phHueSDK.connect(accessPoint);
		// Thread sleep needed because phHueSDK is not instantly ready
		try {
			Thread.sleep(500);
		} catch (InterruptedException ex) {
			
		}
		if (phHueSDK.getSelectedBridge() == null) {
			findBridges();
			
			try {
			Thread.sleep(30000); // 1000 milliseconds is one second.
			// Sleep to give user enough time to push the button
		    } catch (InterruptedException ex) {
			LOG.warn(ex.getMessage());
		}
			
			
			phHueSDK.connect(accessPoint);
		}
		return true;
	}

	/**
	 * Disconnect.
	 *
	 * @return true, if successful
	 */
	public boolean disconnect() {
		PHBridge bridge = phHueSDK.getSelectedBridge();

		phHueSDK.disconnect(bridge);
		return true;
	}

}