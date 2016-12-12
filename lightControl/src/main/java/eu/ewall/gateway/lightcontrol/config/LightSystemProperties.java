package eu.ewall.gateway.lightcontrol.config;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.FileReader;
import java.io.IOException;
import java.util.Properties;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * HueProperties.java
 * 
 * Stores the last known connected IP Address and the last known username. This
 * facilitates automatic bridge connection.
 * 
 * Also, as the username (for the whitelist) is a random string, this prevents
 * the need to pushlink every time the app is started (as the username is read
 * from the properties file).
 *
 */
public final class LightSystemProperties {

	/** The log. */
	private static Logger LOG = LoggerFactory
			.getLogger(LightSystemProperties.class);

	/** The Constant LAST_CONNECTED_IP. */
	private static final String LAST_CONNECTED_IP = "LastIPAddress";

	/** The Constant USER_NAME. */
	private static final String USER_NAME = "WhiteListUsername";

	/** The Constant PROPS_FILE_NAME. */
	private static final String PROPS_FILE_NAME = "phhue-config.properties";

	/** The props. */
	private static Properties props = null;

	/**
	 * Instantiates a new light system properties.
	 */
	private LightSystemProperties() {
	}

	/**
	 * Store last ip address.
	 *
	 * @param ipAddress
	 *            the ip address
	 */
	public static void storeLastIPAddress(String ipAddress) {
		props.setProperty(LAST_CONNECTED_IP, ipAddress);
		saveProperties();
	}

	/**
	 * Stores the Username (for Whitelist usage). This is generated as a random
	 * 16 character string.
	 *
	 * @param username
	 *            the username
	 */
	public static void storeUsername(String username) {
		props.setProperty(USER_NAME, username);
		saveProperties();
	}

	/**
	 * Returns the stored Whitelist username. If it doesn't exist we generate a
	 * 16 character random string and store this in the properties file.
	 *
	 * @return the username
	 */
	public static String getUsername() {
		String username = props.getProperty(USER_NAME);
		return username;
	}

	/**
	 * Gets the last connected ip.
	 *
	 * @return the last connected ip
	 */
	public static String getLastConnectedIP() {
		return props.getProperty(LAST_CONNECTED_IP);
	}

	/**
	 * Load properties.
	 */
	public static void loadProperties() {
		if (props == null) {
			props = new Properties();
			FileInputStream in;

			try {
				in = new FileInputStream(PROPS_FILE_NAME);
				props.load(in);

				in.close();
			} catch (FileNotFoundException ex) {
				saveProperties();
			} catch (IOException e) {
				LOG.warn("Data error ");
			}
		}
	}

	/**
	 * Read properties.
	 */
	public static void readProperties() {
		if (props == null) {
			props = new Properties();

			FileReader read;

			try {
				read = new FileReader(PROPS_FILE_NAME);
				props.load(read);

				read.close();
			} catch (FileNotFoundException ex) {
				saveProperties();
			} catch (IOException e) {
				LOG.warn("Data error ");
			}
		}
	}

	/**
	 * Save properties.
	 */
	public static void saveProperties() {
		try {
			FileOutputStream out = new FileOutputStream(PROPS_FILE_NAME);
			props.store(out, null);
			out.close();
		} catch (FileNotFoundException e) {
			LOG.warn("File not found ");
			// Handle the FileNotFoundException.
		} catch (IOException e) {
			// Handle the IOException.
		}
	}

}