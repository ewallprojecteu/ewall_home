/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.api;

import org.apache.commons.configuration.PropertiesConfiguration;

/**
 * <p>
 * This is an example of an interface that is expected to be implemented by
 * local database provider.
 * </p>
 * 
 * @author emirmos
 */

public interface RemoteProxyDBProvider {

	/**
	 * Intialize db client.
	 *
	 * @param properties the properties
	 * @return true, if successful
	 */
	public boolean intializeDBClient(PropertiesConfiguration properties);

	/**
	 * Gets the version.
	 *
	 * @return the version
	 */
	public String getVersion();

	/**
	 * Close.
	 */
	public void close();

}
