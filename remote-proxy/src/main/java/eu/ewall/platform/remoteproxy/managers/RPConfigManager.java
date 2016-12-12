/*******************************************************************************
 * Copyright 2015 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.managers;

import java.util.HashMap;
import java.util.Iterator;
import java.util.Map;
import java.util.Properties;

import org.apache.commons.configuration.PropertiesConfiguration;
import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.ObjectReader;

import eu.ewall.platform.remoteproxy.api.RemoteProxyProvider;

/**
 * The Class RPConfigManager.
 */
public class RPConfigManager {

	/** The log. */
	private static Logger LOG = LoggerFactory.getLogger(RPConfigManager.class);

	/** The remote proxy provider. */
	private RemoteProxyProvider remoteProxyProvider;

	/**
	 * Instantiates a new RP config manager.
	 *
	 * @param rpp            the rpp
	 */
	public RPConfigManager(RemoteProxyProvider rpp) {
		this.remoteProxyProvider = rpp;
	}

	/**
	 * Gets the remote proxy configuration.
	 *
	 * @return the remote proxy configuration
	 */
	public String getRemoteProxyConfiguration() {

		String json;
		try {
			PropertiesConfiguration props = remoteProxyProvider
					.getConfigurationProperites();

			Map<String, String> hash = new HashMap<String, String>();
			@SuppressWarnings("unchecked")
			Iterator<String> keys = props.getKeys();
			while (keys.hasNext()) {
				String key = keys.next();
				hash.put(key, props.getString(key));
			}

			// serialize HashMap to json
			ObjectMapper objectMapper = new ObjectMapper();
			json = objectMapper.writeValueAsString(hash);

		} catch (Exception e) {
			LOG.error("Error while getting RP configuration", e);
			return null;
		}

		return json;
	}

	/**
	 * Sets the remote proxy configuration.
	 *
	 * @param newConfiguration
	 *            the config
	 * @return the response
	 */

	public boolean setRemoteProxyConfiguration(String newConfiguration) {

		PropertiesConfiguration oldProperties = remoteProxyProvider
				.getConfigurationProperites();

		try {
			ObjectMapper mapper = new ObjectMapper();
			ObjectReader objectReader = mapper.reader(Properties.class);
			Properties newProperties = (Properties) objectReader
					.readValue(newConfiguration);

			PropertiesConfiguration newPropertiesConfiguration = new PropertiesConfiguration();

			// convert from Properties to PropertiesConfiguration
			for (Object key : newProperties.keySet()) {
				newPropertiesConfiguration.setProperty(key.toString(),
						newProperties.getProperty((String) key));
			}

			// load and save properties to remote proxy
			remoteProxyProvider
					.setConfigurationProperites(newPropertiesConfiguration);

			// TODO: test restarting (possible solution is running restart in
			// the new thread)
			if (arePropertiesChanged(oldProperties, newPropertiesConfiguration)) {
				// if there was changes in properties, restart remote proxy
				if (remoteProxyProvider.restart(oldProperties)) {
					LOG.debug("New remote proxy configuration successfully stored");
				} else {
					LOG.debug("New remote proxy configuration cannot be stored");
				}
			}

		} catch (Exception e) {
			LOG.error("Error while saving new remote proxy configuration, loading old properties", e);
			remoteProxyProvider.setConfigurationProperites(oldProperties);
			return false;
		}

		return true;
	}

	/**
	 * Are properties changed.
	 *
	 * @param oldProperties
	 *            the old properties
	 * @param newProperties
	 *            the new properties
	 * @return true, if successful
	 */
	private boolean arePropertiesChanged(PropertiesConfiguration oldProperties,
			PropertiesConfiguration newProperties) {

		// @SuppressWarnings("unchecked")
		// List<String> oldKeys = Lists.newArrayList(oldProperties.getKeys());
		// Collections.sort(oldKeys);
		// @SuppressWarnings("unchecked")
		// List<String> newKeys = Lists.newArrayList(newProperties.getKeys());
		// Collections.sort(newKeys);
		//
		// if (oldKeys.size() != newKeys.size()) {
		// return true;
		// }
		//
		// int listSize = oldKeys.size();
		//
		// for (int i = 0; i < listSize; i++) {
		// if (!oldProperties.getString(oldKeys.get(i)).equals(
		// newProperties.getString(newKeys.get(i)))) {
		// return true;
		// }
		// }
		//
		// return false;

		return true;
	}

}
