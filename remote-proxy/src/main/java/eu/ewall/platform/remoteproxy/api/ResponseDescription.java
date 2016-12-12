/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.api;

/**
 * The Class ResponseDescription.
 *
 * @author emirmos
 */
public class ResponseDescription {

	/** Response status code. */
	private RequestStatus requestStatusCode;

	/** Created resource uri. */
	private String resourceURI;

	/**
	 * Instantiates a new response description.
	 */
	public ResponseDescription() {
		super();
		// TODO Auto-generated constructor stub
	}

	/**
	 * Instantiates a new response description.
	 *
	 * @param statusCode
	 *            the status code
	 */
	public ResponseDescription(RequestStatus statusCode) {
		super();
		this.requestStatusCode = statusCode;
	}

	/**
	 * Instantiates a new response description.
	 *
	 * @param statusCode
	 *            the status code
	 * @param resourceURI
	 *            the resource uri
	 */
	public ResponseDescription(RequestStatus statusCode, String resourceURI) {
		super();
		this.requestStatusCode = statusCode;
		this.resourceURI = resourceURI;
	}

	/**
	 * Gets the status code.
	 *
	 * @return the statusCode
	 */
	public RequestStatus getStatusCode() {
		return requestStatusCode;
	}

	/**
	 * Sets the status code.
	 *
	 * @param statusCode
	 *            the statusCode to set
	 */
	public void setStatusCode(RequestStatus statusCode) {
		this.requestStatusCode = statusCode;
	}

	/**
	 * Gets the resource uri.
	 *
	 * @return the resourceURI
	 */
	public String getResourceURI() {
		return resourceURI;
	}

	/**
	 * Sets the resource uri.
	 *
	 * @param resourceURI
	 *            the resourceURI to set
	 */
	public void setResourceURI(String resourceURI) {
		this.resourceURI = resourceURI;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "ResponseDescription [statusCode = " + requestStatusCode
				+ ", resourceURI = " + resourceURI + "]";
	}

}
