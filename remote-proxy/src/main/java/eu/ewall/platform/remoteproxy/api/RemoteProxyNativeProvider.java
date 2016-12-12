/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.api;

/**
 * Represents most basic native interface planned to be used by C++ components
 * in home environment. Advance interface is provided with
 * {@link RemoteProxyProvider}.
 * 
 * @author emirmos
 * 
 */
public interface RemoteProxyNativeProvider {

	/**
	 * Simple push of data to cloud data repository.
	 *
	 * @param deviceControllerId
	 *            the device controller id
	 * @param data
	 *            the data
	 * @return true, if successful
	 */
	public boolean pushData(String deviceControllerId, String data);

	/**
	 * Simple update of data stored on cloud data repository.
	 *
	 * @param deviceControllerId
	 *            the device controller id
	 * @param dataId
	 *            the data id
	 * @param data
	 *            the data
	 * @return true, if successful
	 */
	public boolean updateData(String deviceControllerId, String dataId,
			String data);

}
