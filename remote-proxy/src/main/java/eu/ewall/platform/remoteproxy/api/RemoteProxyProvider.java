/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.api;

import org.apache.commons.configuration.PropertiesConfiguration;

import eu.ewall.platform.commons.datamodel.ewallsystem.ProxyStatus;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;
import eu.ewall.platform.remoteproxy.config.RemoteProxyConfig;

/**
 * <p>
 * This is an interface for a provider of Remote Proxy functionality. Remote
 * Proxy handles communication between the local and cloud environments, in
 * order to provide security, capacity, availability and reliability.
 * </p>
 * 
 * @author emirmos
 */
public interface RemoteProxyProvider extends RemoteProxyNativeProvider {

	/**
	 * In order to provide security, before an RemoteProxyConsumer entity (local
	 * service component, device controller, etc.) can use RemoteProxyProvider
	 * (i.e. to send or retrieve data from cloud) it has to register to eWALL
	 * system.
	 *
	 * @return true, if successful
	 */
	public boolean doRegister();

	/**
	 * This is main method that handles Remote Proxy synchronous requests.
	 * Request Description contains information on type of request (create,
	 * retrieve, update or delete a resource), id of requesting entity (remote
	 * proxy consumer), resource representation and optional request parameters
	 *
	 * @param request
	 *            the request
	 * @return the response description
	 * @see RequestDescription
	 */
	public ResponseDescription doRequest(RequestDescription request);

	/**
	 * Support for asynchronous publish/subscribe mechanism. Used when remote
	 * proxy consumer entity wants to asynchronous sent data to eWALL cloud.
	 *
	 * @param remoteProxyConsumerId
	 *            the remote proxy consumer id
	 * @param resourceRepresentation
	 *            the resource representation
	 * @return true, if successful
	 */
	public boolean doPublish(String remoteProxyConsumerId,
			String resourceRepresentation);

	/**
	 * Support for asynchronous publish/subscribe mechanism. Used when remote
	 * proxy consumer entity wants to receive asynchronous events from eWALL
	 * cloud (i.e. in resource change). Method processNotify form
	 * RemoteProxyConsumer is called when such event occurs. TODO create and use
	 * Subscribe class
	 *
	 * @param remoteProxyConsumerId
	 *            the remote proxy consumer id
	 * @param subscribeRequest
	 *            the subscribe request
	 * @return true, if successful
	 */
	public boolean doSubscribe(String remoteProxyConsumerId,
			String subscribeRequest);

	/**
	 * Starts Remote Proxy. Performs registration of a local sensing environment
	 * to eWALL Cloud platform and starts listening for requests.
	 * 
	 * @return true if successful, false if fail or error
	 */
	public boolean startup();

	/**
	 * Stops Remote Proxy. Performs de-registration of a local sensing
	 * environment from eWALL Cloud platform and stops receiving and sending
	 * requests.
	 * 
	 * @return true if successful, false if fail or error
	 */
	public boolean shutdown();

	/**
	 * Connects Remote Proxy to eWALL Cloud.
	 * 
	 * @return true if successful, false if fail or error
	 */
	public boolean connect();

	/**
	 * Disconnect Remote Proxy from eWALL Cloud.
	 * 
	 * @return true if successful, false if fail or error
	 */
	public boolean disconnect();

	/**
	 * Gets current state of RemoteProxy.
	 *
	 * @return the status
	 */
	public ProxyStatus getStatus();

	/**
	 * Sets configuration properties. May cause proxy restart.
	 *
	 * @param configrationProperties
	 *            the configration properties
	 * @return true, if successful
	 */
	public boolean setConfigurationProperites(
			PropertiesConfiguration configrationProperties);


	/**
	 * /** Gets the configuration properites.
	 *
	 * @return the configuration properites
	 */
	public PropertiesConfiguration getConfigurationProperites();

	/**
	 * Restart remote proxy. Method will try to start remote proxy with current
	 * (usually new) properties and (if it fails) start properties with old
	 * working properties
	 *
	 * @param oldWorkingProperties
	 *            the old working properties
	 * @return true, if successfully started with new properties
	 */
	public boolean restart(PropertiesConfiguration oldWorkingProperties);
	
	
	public RemoteProxyConfig getConfigManager();
	
	public RPCloudClient getRPCloudClient();
	
	public void setRemoteProxyDBProvider(RemoteProxyDBProvider remoteProxyDBProvider);

}
