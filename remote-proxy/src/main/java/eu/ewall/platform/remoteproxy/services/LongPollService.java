/*******************************************************************************
 * Copyright 2015 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.services;

import java.io.BufferedReader;
import java.io.IOException;
import java.io.InputStreamReader;
import java.io.OutputStream;
import java.net.HttpURLConnection;
import java.net.MalformedURLException;
import java.net.URL;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import com.fasterxml.jackson.databind.ObjectMapper;
import com.fasterxml.jackson.databind.ObjectReader;
import com.fasterxml.jackson.databind.ObjectWriter;

import eu.ewall.platform.commons.datamodel.ewallsystem.ActuatorCommand;
import eu.ewall.platform.commons.datamodel.ewallsystem.LongPollMessage;
import eu.ewall.platform.commons.datamodel.ewallsystem.LongPollMessageType;
import eu.ewall.platform.remoteproxy.api.RemoteProxyProvider;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;
import eu.ewall.platform.remoteproxy.managers.RPActuatorControlManager;
import eu.ewall.platform.remoteproxy.managers.RPConfigManager;

/**
 * The Class LongPollService.
 */
public class LongPollService implements Runnable {

	/** The log. */
	private static final Logger LOG = LoggerFactory
			.getLogger(LongPollService.class);

	/** The update url. */
	private String updateURL;

	/** The rest client. */
	private RPCloudClient restClient;

	/** The remote proxy provider. */
	private RemoteProxyProvider remoteProxyProvider;

	/** The connection. */
	private HttpURLConnection connection;

	/** The thread. */
	private Thread thread;

	/** The rp actuator control manager. */
	private RPActuatorControlManager rpActuatorControlManager;

	/** The rp config manager. */
	private RPConfigManager rpConfigManager;

	/** The timeout for connecting to cloud gateway. (only for connecting) */
	private static final int CONNECTION_TIMEOUT = 15 * 1000; // 15 sec

	/**
	 * The timeout for long polling waiting. Must be greater than
	 * eu.ewall.platform
	 * .middleware.cloudgateway.services.RemoteProxyLongPollService
	 * .LONG_POLL_TIMEOUT 3 minute + additional 5 seconds If there is no new
	 * data in period of 3 minute, cloud gateway will return message
	 * NO_NEW_REQUESTS.
	 */
	private static final int LONG_POLL_TIMEOUT = (1 * 60 * 1000) + 5000;

	/** The sleep timeout after failed connection to cloud gateway. */
	private static final int FAILED_CONNECTION_SLEEP_TIMEOUT = 10 * 1000;

	/**
	 * Instantiates a new long poll service.
	 *
	 * @param updateURL            the update url
	 * @param restClient            the rest client
	 * @param rpp            the rpp
	 */
	public LongPollService(String updateURL, RPCloudClient restClient,
			RemoteProxyProvider rpp) {
		this.updateURL = updateURL;
		this.restClient = restClient;
		this.remoteProxyProvider = rpp;

		this.rpActuatorControlManager = new RPActuatorControlManager();
		this.rpConfigManager = new RPConfigManager(rpp);
	}

	/**
	 * Start long polling.
	 */
	public void startLongPolling() {
		thread = new Thread(this);
		thread.start();
	}

	/**
	 * Stop long polling.
	 */
	public void stopLongPolling() {
		LOG.info("Stopping long polling thread...");
		if (thread != null) {
			try {
				if (connection != null) {
					connection.disconnect();
				}
				thread.join(100);
				LOG.info("Long polling thread stopped...");
			} catch (Exception e) {
				LOG.error(e.getMessage());
			}
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see java.lang.Runnable#run()
	 */
	@Override
	public void run() {

		// initial message, no data to cloud gateway
		LongPollMessage requestMessage = new LongPollMessage(Long.valueOf(0),
				LongPollMessageType.REQUEST_WITHOUT_DATA, "");

		while (true) {

			try {
				// logger.debug("Sending long poll request to cloud gateway...");
				connection = (HttpURLConnection) new URL(updateURL)
						.openConnection();
				connection.setRequestMethod("POST");

				connection.setConnectTimeout(CONNECTION_TIMEOUT); // connecting
																	// timeout
				connection.setReadTimeout(LONG_POLL_TIMEOUT); // long pooling
																// timeout
																// (waiting for
																// new messages)

				connection.setDoOutput(true);

				// send remote proxy data to cloud gateway if it was
				// requested
				ObjectWriter ow = new ObjectMapper().writer()
						.withDefaultPrettyPrinter();
				String requestMessagejson = ow
						.writeValueAsString(requestMessage);
				connection.setRequestProperty("Content-Type",
						"application/json; charset=utf-8");
				connection.setRequestProperty("Content-Length",
						String.valueOf(requestMessagejson.length()));

				// add auth token
				connection.setRequestProperty("X-Auth-Token",
						restClient.getAuthToken());

				OutputStream os = connection.getOutputStream();
				os.write(requestMessagejson.getBytes());
				os.close();

				BufferedReader in = new BufferedReader(new InputStreamReader(
						connection.getInputStream()));

				// GET RESPONSE DATA from cloud gateway
				StringBuffer response = new StringBuffer();
				String inputLine;
				while ((inputLine = in.readLine()) != null) {
					response.append(inputLine);
				}
				String responseMessageJson = response.toString();
				in.close();

				ObjectMapper mapper = new ObjectMapper();
				ObjectReader objectReader = mapper
						.reader(LongPollMessage.class);
				LongPollMessage responseMessage = (LongPollMessage) objectReader
						.readValue(responseMessageJson);

				requestMessage = handleResponseMessage(responseMessage);
			}

			catch (MalformedURLException e) {
				LOG.debug(e.getMessage());
				break;
			} catch (IOException e) {
				LOG.debug(e.getMessage());

				if (e.getMessage().startsWith(
						"Server returned HTTP response code: 401")) {
					restClient.login();
				} else {
					try {
						// sleep after failed connection
						Thread.sleep(FAILED_CONNECTION_SLEEP_TIMEOUT);
					} catch (InterruptedException e1) {
					}
				}
			}
		}

	}

	/**
	 * Handle response message.
	 * <p>
	 * Messages type: NO_NEW_REQUESTS- no new requests from ewall portal
	 * </p>
	 * <p>
	 * GET_STATUS- remote proxy status must be sent (in next long poll
	 * iteration) to ewall portal
	 * </p>
	 * <p>
	 * GET_CONFIG- remote proxy configuration must be sent (in next long poll
	 * iteration) to ewall portal
	 * </p>
	 * <p>
	 * SET_CONFIG- ewall send new configuration that must be stored in remote
	 * proxy
	 * </p>
	 * <p>
	 * UPDATE_ACTUATOR_COMMAND- ewall send actuator command to remote proxy
	 * </p>
	 *
	 * @param responseMessage
	 *            the response message
	 * @return the long poll message
	 */
	LongPollMessage handleResponseMessage(LongPollMessage responseMessage) {

		// instance default message
		LongPollMessage requestMessage = new LongPollMessage(Long.valueOf(0),
				LongPollMessageType.REQUEST_WITHOUT_DATA, "");

		switch (responseMessage.getActionType()) {
		case NO_NEW_REQUESTS:
			// jump over (it must be checked for algorithm correct work)
			break;

		case GET_CONFIG:
			requestMessage = new LongPollMessage(
					responseMessage.getRequestID(),
					LongPollMessageType.REQUEST_WITH_DATA,
					rpConfigManager.getRemoteProxyConfiguration());

			// remote proxy CONFIG will be delivered in next POST request
			break;

		case SET_CONFIG:
			requestMessage = new LongPollMessage(
					responseMessage.getRequestID(),
					LongPollMessageType.REQUEST_WITH_DATA, "");
			rpConfigManager.setRemoteProxyConfiguration(responseMessage
					.getMessageData());
			break;

		case UPDATE_ACTUATOR_COMMAND:
			requestMessage = new LongPollMessage(
					responseMessage.getRequestID(),
					LongPollMessageType.REQUEST_WITH_DATA, "");
			ObjectMapper mapper = new ObjectMapper();
			try {
				ActuatorCommand actuatorCommand = mapper.reader(
						ActuatorCommand.class).readValue(
						responseMessage.getMessageData());
				rpActuatorControlManager.executeCommand(actuatorCommand);
			} catch (Exception e) {
				LOG.error("Error in getting actuator command", e);
			}
			break;

		default:
			LOG.error("Wrong data received!, Response message = ", responseMessage);
			break;
		}

		return requestMessage;
	}

	/**
	 * Gets the update url.
	 *
	 * @return the update url
	 */
	public String getUpdateURL() {
		return updateURL;
	}

	/**
	 * Sets the update url.
	 *
	 * @param updateURL
	 *            the new update url
	 */
	public void setUpdateURL(String updateURL) {
		this.updateURL = updateURL;
	}

	/**
	 * Gets the rest client.
	 *
	 * @return the rest client
	 */
	public RPCloudClient getRestClient() {
		return restClient;
	}

	/**
	 * Sets the rest client.
	 *
	 * @param restClient
	 *            the new rest client
	 */
	public void setRestClient(RPCloudClient restClient) {
		this.restClient = restClient;
	}

	/**
	 * Gets the remote proxy provider.
	 *
	 * @return the remote proxy provider
	 */
	public RemoteProxyProvider getRemoteProxyProvider() {
		return remoteProxyProvider;
	}

	/**
	 * Sets the remote proxy provider.
	 *
	 * @param remoteProxyProvider
	 *            the new remote proxy provider
	 */
	public void setRemoteProxyProvider(RemoteProxyProvider remoteProxyProvider) {
		this.remoteProxyProvider = remoteProxyProvider;
	}
}
