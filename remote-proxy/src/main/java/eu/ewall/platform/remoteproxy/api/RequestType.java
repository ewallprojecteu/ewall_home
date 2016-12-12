/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.api;

/**
 * Type of request to be handled by Remote Proxy.
 * 
 * @author emirmos
 * 
 */
public enum RequestType {

	/** The create. */
	CREATE,
	/** The retrieve. */
	RETRIEVE,
	/** The update. */
	UPDATE,
	/** The delete. */
	DELETE;

	/**
	 * Value.
	 *
	 * @return the string
	 */
	public String value() {
		return name();
	}

	/**
	 * Value.
	 *
	 * @param v
	 *            the v
	 * @return the request type
	 */
	public static RequestType value(String v) {
		return valueOf(v);
	}

}
