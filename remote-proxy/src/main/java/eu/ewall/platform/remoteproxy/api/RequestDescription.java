/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.api;

import java.util.List;
import java.util.Map;

/**
 * The Class RequestDescription.
 *
 * @author emirmos
 */

public class RequestDescription {

	/** Type of request: create, retrieve, update, delete. */
	private RequestType type;

	/**
	 * The id of local entity (device controller, service component,
	 * application) performing a request.
	 */
	private String remoteProxyConsumerId;

	/** resource representation. */
	private String resourceRepresentation;

	/** Additional request parameters. */
	private Map<String, List<String>> parameters;

	/**
	 * Gets the type.
	 *
	 * @return the type
	 */
	public RequestType getType() {
		return type;
	}

	/**
	 * Sets the type.
	 *
	 * @param type
	 *            the type to set
	 */
	public void setType(RequestType type) {
		this.type = type;
	}

	/**
	 * Gets the remote proxy consumer id.
	 *
	 * @return the remoteProxyConsumerId
	 */
	public String getRemoteProxyConsumerId() {
		return remoteProxyConsumerId;
	}

	/**
	 * Sets the remote proxy consumer id.
	 *
	 * @param remoteProxyConsumerId
	 *            the remoteProxyConsumerId to set
	 */
	public void setRemoteProxyConsumerId(String remoteProxyConsumerId) {
		this.remoteProxyConsumerId = remoteProxyConsumerId;
	}

	/**
	 * Gets the resource representation.
	 *
	 * @return the resourceRepresentation
	 */
	public String getResourceRepresentation() {
		return resourceRepresentation;
	}

	/**
	 * Sets the resource representation.
	 *
	 * @param resourceRepresentation
	 *            the resourceRepresentation to set
	 */
	public void setResourceRepresentation(String resourceRepresentation) {
		this.resourceRepresentation = resourceRepresentation;
	}

	/**
	 * Gets the parameters.
	 *
	 * @return the parameters
	 */
	public Map<String, List<String>> getParameters() {
		return parameters;
	}

	/**
	 * Sets the parameters.
	 *
	 * @param parameters
	 *            the parameters to set
	 */
	public void setParameters(Map<String, List<String>> parameters) {
		this.parameters = parameters;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Object#toString()
	 */
	@Override
	public String toString() {
		return "RequestDescription [type = " + type
				+ ", remoteProxyConsumerId = " + remoteProxyConsumerId
				+ ", resourceRepresentation = " + resourceRepresentation
				+ ", parameters = " + parameters + "]";
	}

	/**
	 * Instantiates a new request description.
	 *
	 * @param type
	 *            the type
	 * @param requestingEntityId
	 *            the requesting entity id
	 * @param resourceRepresentation
	 *            the resource representation
	 * @param parameters
	 *            the parameters
	 */
	public RequestDescription(RequestType type, String requestingEntityId,
			String resourceRepresentation, Map<String, List<String>> parameters) {
		super();
		this.type = type;
		this.remoteProxyConsumerId = requestingEntityId;
		this.resourceRepresentation = resourceRepresentation;
		this.parameters = parameters;
	}

	/**
	 * Instantiates a new request description.
	 */
	public RequestDescription() {
	}

}
