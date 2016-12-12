/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.api;

/**
 * <p>
 * This is an example of an interface that is expected to be implemented by
 * Consumers of the API; for example this interface may define a listener or a
 * callback. Adding methods to this interface is a MAJOR change, because ALL
 * clients are affected.
 * </p>
 * 
 * @author emirmos
 */

public interface RemoteProxyConsumer {

	/**
	 * TODO create and use Notification class.
	 *
	 * @param notification
	 *            the notification
	 */
	public void processNotification(String notification);

	/**
	 * Returns RemoteProxyConsumer id.
	 *
	 * @return the remote proxy consumer id
	 */
	public String getRemoteProxyConsumerId();

}
