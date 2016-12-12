/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.api;

/**
 * The Enum RequestStatus.
 *
 * @author emirmos
 */
public enum RequestStatus {

	// Based on HTTP Status Codes.
	/** The status ok. */
	STATUS_OK,
	/** The status created. */
	STATUS_CREATED,
	/** The status accepted. */
	STATUS_ACCEPTED,
	/** The status bad request. */
	STATUS_BAD_REQUEST,
	/** The status unauthorized. */
	STATUS_UNAUTHORIZED,
	/** The status forbidden. */
	STATUS_FORBIDDEN,
	/** The status not found. */
	STATUS_NOT_FOUND,
	/** The status method not allowed. */
	STATUS_METHOD_NOT_ALLOWED,
	/** The status not acceptable. */
	STATUS_NOT_ACCEPTABLE,
	/** The status request timeout. */
	STATUS_REQUEST_TIMEOUT,
	/** The status conflict. */
	STATUS_CONFLICT,
	/** The status internal server error. */
	STATUS_INTERNAL_SERVER_ERROR,
	/** The status not implemented. */
	STATUS_NOT_IMPLEMENTED,
	/** The status service unavailable. */
	STATUS_SERVICE_UNAVAILABLE,

	// Remote Proxy specific status
	/** The status proxy error. */
	STATUS_PROXY_ERROR,
	/** The status proxy busy. */
	STATUS_PROXY_BUSY;

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
	 * @return the request status
	 */
	public static RequestStatus value(String v) {
		return valueOf(v);
	}

}