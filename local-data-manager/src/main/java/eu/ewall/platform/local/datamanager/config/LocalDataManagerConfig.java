/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.local.datamanager.config;

import org.apache.commons.configuration.PropertiesConfiguration;

/**
 * Represents interface to access configuration properties.
 *
 * @author emirmos
 */
public interface LocalDataManagerConfig {

	/**
	 * Gets the properties.
	 *
	 * @return the properties
	 */
	PropertiesConfiguration getProperties();

	/**
	 * Load the properties.
	 *
	 * @return the properties
	 */
	PropertiesConfiguration loadProperties();

	/**
	 * Sets the properties.
	 *
	 * @param prop
	 *            the prop
	 * @return true, if successful
	 */
	boolean setProperties(PropertiesConfiguration prop);

	/**
	 * Gets the config file name.
	 *
	 * @return the config file name
	 */
	String getConfigFileName();

}
