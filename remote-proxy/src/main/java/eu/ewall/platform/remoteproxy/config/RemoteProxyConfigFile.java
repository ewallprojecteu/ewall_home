/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.config;

import java.io.FileInputStream;
import java.io.FileNotFoundException;
import java.io.FileOutputStream;
import java.io.IOException;
import java.io.InputStream;
import java.io.OutputStream;
import java.util.Iterator;

import org.apache.commons.configuration.ConfigurationException;
import org.apache.commons.configuration.PropertiesConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

/**
 * Handles configuration properties form local file in project root folder.
 * 
 * @author emirmos
 * 
 */
public class RemoteProxyConfigFile implements RemoteProxyConfig {

	// properties are in project root
	/** The config file. */
	private String configFile = "./config.properties";

	/** The prop. */
	PropertiesConfiguration prop = new PropertiesConfiguration();

	/** The logger. */
	Logger logger = LoggerFactory.getLogger(RemoteProxyConfigFile.class);

	/**
	 * Instantiates a new remote proxy config file.
	 */
	public RemoteProxyConfigFile() {
		loadProperties();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.config.RemoteProxyConfig#getProperties()
	 */
	@Override
	public PropertiesConfiguration getProperties() {
		return prop;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.config.RemoteProxyConfig#setProperties(
	 * java.util.Properties)
	 */
	@Override
	public boolean setProperties(PropertiesConfiguration newProperties) {

		OutputStream output = null;

		try {
			output = new FileOutputStream(configFile);

			// rewrite old properties with new properties
			// use old properties object to keep layout of "config.properties"
			// file
			@SuppressWarnings("unchecked")
			Iterator<String> keys = newProperties.getKeys();
			while (keys.hasNext()) {
				String key = keys.next();

				if (key.equals("eu.ewall.platform.remoteproxy.sensingenvironment.id"))
					continue; // dont change uuid

				prop.setProperty(key, newProperties.getProperty(key));
			}

			// store properties file
			prop.save(output);

			logger.info("Successfuly stored remote proxy configuration properties to file "
					+ configFile + ".");
			return true;

		} catch (ConfigurationException | FileNotFoundException e) {
			logger.error("Error in storing configuration properties to file", e);
			return false;
		} finally {
			if (output != null) {
				try {
					output.close();
				} catch (IOException e) {
					logger.error("Error in closing output file stream", e);
				}
			}

		}

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.config.RemoteProxyConfig#getConfigFileName
	 * ()
	 */
	@Override
	public String getConfigFileName() {
		return configFile;
	}

	@Override
	public PropertiesConfiguration loadProperties() {
		prop = new PropertiesConfiguration();
		InputStream input = null;

		try {
			input = new FileInputStream(configFile);

			// load a properties file
			prop.load(input);

			logger.info("Successfuly loaded remote proxy configuration properties from file "
					+ configFile + ".");

		} catch (Exception ex) {
			logger.error("Error in reading configuration properties from file", ex);
		} finally {
			if (input != null) {
				try {
					input.close();
				} catch (IOException e) {
					logger.error("Error in closing input file stream", e);
				}
			}
		}
		return prop;
	}

}
