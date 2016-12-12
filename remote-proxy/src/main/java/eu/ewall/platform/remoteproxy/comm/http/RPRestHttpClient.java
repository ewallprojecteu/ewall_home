/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.comm.http;

import java.util.List;

import javax.ws.rs.client.Client;
import javax.ws.rs.client.ClientBuilder;
import javax.ws.rs.client.Entity;
import javax.ws.rs.client.Invocation;
import javax.ws.rs.client.WebTarget;
import javax.ws.rs.core.MediaType;
import javax.ws.rs.core.Response;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;
import org.springframework.core.ParameterizedTypeReference;
import org.springframework.http.HttpEntity;
import org.springframework.http.HttpHeaders;
import org.springframework.http.HttpMethod;
import org.springframework.http.HttpStatus;
import org.springframework.http.ResponseEntity;
import org.springframework.web.client.HttpClientErrorException;
import org.springframework.web.client.HttpServerErrorException;
import org.springframework.web.client.ResourceAccessException;
import org.springframework.web.client.RestTemplate;

import com.google.gson.JsonElement;
import com.google.gson.JsonObject;
import com.google.gson.JsonParser;
import com.google.gson.JsonSyntaxException;

import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.commons.datamodel.device.DeviceType;
import eu.ewall.platform.commons.datamodel.ewallsystem.PointOfContact;
import eu.ewall.platform.commons.datamodel.marshalling.json.DM2JsonObjectMapper;
import eu.ewall.platform.commons.datamodel.message.Notification;
import eu.ewall.platform.commons.datamodel.profile.preferences.SystemPreferences;
import eu.ewall.platform.remoteproxy.api.MethodResult;
import eu.ewall.platform.remoteproxy.comm.RPCloudClient;

/**
 * The Class RPRestHttpClient.
 *
 */
public class RPRestHttpClient implements RPCloudClient {

	/** The logger. */
	Logger logger = LoggerFactory.getLogger(RPRestHttpClient.class);

	/** The e wall system root. */
	private String eWallSystemRoot;

	/** The sensing environment id. */
	private String sensingEnvironmentId;

	/** The auth token. */
	private String authToken = "";

	/** The username. */
	private String username;

	/** The password. */
	private String password;

	/** The is registered. */
	private boolean isRegistered = false;

	/**
	 * Instantiates a new RP rest http client.
	 *
	 * @param eWallSystemRoot
	 *            the e wall system root
	 * @param sensingEnvironmentId
	 *            the sensing environment id
	 * @param passphrase
	 *            the passphrase
	 */
	public RPRestHttpClient(String eWallSystemRoot, String sensingEnvironmentId) {
		super();
		this.eWallSystemRoot = eWallSystemRoot;
		this.sensingEnvironmentId = sensingEnvironmentId;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#register(eu.ewall.platform
	 * .commons.datamodel.ewallsystem.PointOfContact)
	 */
	@Override
	public synchronized MethodResult register(PointOfContact poc) {

		logger.debug("Sending register message to eWALL cloud");

		HttpHeaders headers = new HttpHeaders();
		headers.set("X-Auth-Token", this.authToken);
		// headers.setContentType(org.springframework.http.MediaType.APPLICATION_JSON);

		HttpEntity<String> entity = new HttpEntity<String>(
				DM2JsonObjectMapper.writeValueAsString(poc), headers);

		RestTemplate restTemplate = new RestTemplate();
		ResponseEntity<String> responseBody;

		String path = eWallSystemRoot + "cloud-gateway/sensingenvironments/"
				+ sensingEnvironmentId + "/register";

		if (poc == null) {
			this.isRegistered = false;
			logger.warn("Trying to register with null point of contact information");
			return MethodResult.ERROR;
		}

		/*
		 * Note: not a clean way to do logic related to registering sensing
		 * environment, but it works - logic used in deregister method was
		 * problematic here because of handling "405 Method Not Allowed"
		 * response
		 */
		try {
			responseBody = restTemplate.exchange(path, HttpMethod.POST, entity,
					new ParameterizedTypeReference<String>() {
					});

			logger.debug("Received response status = {}",
					responseBody.getStatusCode());

			if (responseBody.getStatusCode() == HttpStatus.OK) {
				this.isRegistered = true;
				return MethodResult.SUCCESS;
			}

			this.isRegistered = false;
			return MethodResult.ERROR;

		} catch (Exception outerException) {
			if (outerException.getMessage().equals("401 Unauthorized")) {
				// sensing environment is not enabled (disabled)
				this.isRegistered = false;
				return MethodResult.NOT_AUTHORIZED;

			} else if (outerException.getMessage().equals(
					"405 Method Not Allowed")) {
				// sensing environment is not enabled (disabled)
				this.isRegistered = false;
				return MethodResult.NOT_ALLOWED;
			} else if (outerException.getMessage().equals("406 Not Acceptable")) {
				// local-platform version not equals to required one
				this.isRegistered = false;
				return MethodResult.NOT_REQUIRED_VERSION;
			}

			logger.error("Error in sending register message to cloud",
					outerException);
			this.isRegistered = false;
			return MethodResult.ERROR;
		}
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#deRegister(eu.ewall.
	 * platform.commons.datamodel.resource.PointOfContact)
	 */
	@Override
	public synchronized boolean deRegister(PointOfContact poc) {
		logger.debug("Sending de-register message to eWALL cloud");

		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/deregister");

			if (poc != null) {
				Response response = resourceWebTarget
						.request()
						.header("X-Auth-Token", this.authToken)
						.post(Entity.entity(
								DM2JsonObjectMapper.writeValueAsString(poc),
								MediaType.TEXT_PLAIN));

				logger.debug("Received response status = {}",
						Integer.valueOf(response.getStatus()));

				if (response.getStatus() == Response.Status.OK.getStatusCode()) {
					this.isRegistered = false;
					return true;
				} else if (response.getStatus() == Response.Status.UNAUTHORIZED
						.getStatusCode()) {
					// trying to login again
					if (login()) {
						// if successful
						response = resourceWebTarget
								.request()
								.headers(null)
								.header("X-Auth-Token", this.authToken)
								.post(Entity.entity(DM2JsonObjectMapper
										.writeValueAsString(poc),
										MediaType.TEXT_PLAIN));

						logger.debug("Received response status = {}",
								Integer.valueOf(response.getStatus()));

						if (response.getStatus() == Response.Status.OK
								.getStatusCode()) {
							logger.debug("Successfully deregistered");
							this.isRegistered = false;
							return true;
						}

					}
				}

			} else {
				Response response = resourceWebTarget.request()
						.header("X-Auth-Token", this.authToken)
						.post(Entity.entity("null", MediaType.TEXT_PLAIN));

				logger.debug("Received response status = {}",
						Integer.valueOf(response.getStatus()));

				if (response.getStatus() == Response.Status.OK.getStatusCode()) {
					this.isRegistered = false;
					return true;
				} else if (response.getStatus() == Response.Status.UNAUTHORIZED
						.getStatusCode()) {
					// trying to login again
					if (login()) {
						// if successful
						response = resourceWebTarget
								.request()
								.headers(null)
								.header("X-Auth-Token", this.authToken)
								.post(Entity.entity("null",
										MediaType.TEXT_PLAIN));
						logger.debug("Received response status = {}",
								Integer.valueOf(response.getStatus()));

						if (response.getStatus() == Response.Status.OK
								.getStatusCode()) {
							logger.debug("Successfully deregistered");
							this.isRegistered = false;
							return true;
						}

					}
				}

			}

			client.close();

		} catch (Exception e) {
			logger.error(e.toString());
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.comm.RPCloudClient#getDeviceInfo()
	 */
	@Override
	public synchronized List<Device> getDeviceInfo() {
		logger.debug("Retriving device info from eWall cloud");

		try {
			HttpHeaders headers = new HttpHeaders();
			headers.set("X-Auth-Token", this.authToken);

			HttpEntity<Void> entity = new HttpEntity<Void>(null, headers);

			RestTemplate restTemplate = new RestTemplate();

			String path = eWallSystemRoot
					+ "cloud-gateway/sensingenvironments/"
					+ sensingEnvironmentId + "/devices?sensor=true";

			ResponseEntity<List<Device>> response = restTemplate.exchange(path,
					HttpMethod.GET, entity,
					new ParameterizedTypeReference<List<Device>>() {
					});

			if (response.getStatusCode().is2xxSuccessful()) {
				List<Device> devices = response.getBody();
				logger.info(
						"Successfully received device info for {} devices.",
						Integer.valueOf(devices.size()));
				return devices;
			}

		} catch (Exception e) {
			logger.error("Error in retriving device info from eWall cloud", e);

		}
		return null;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPRestClient#doPost(java.lang.String,
	 * java.lang.String)
	 */
	@Override
	public synchronized boolean doPost(String deviceId, String content) {

		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to POST data");
			return false;
		}

		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/devices/" + deviceId
							+ "/environmental/add");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(content));
			// logger.debug("response status = " + response.getStatus());

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				client.close();
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				logger.debug("Sending environmental data resulted in response status UNAUTHORIZED. Trying to login and then again");
				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(content));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						client.close();
						return true;
					}

				}
			}

			client.close();
			logger.debug("Sending environmental data was not sucessfull");

		} catch (Exception e) {
			logger.error("Error in sending evnironmental data", e);
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPRestClient#doPut(java.lang.String,
	 * java.lang.String, java.lang.String)
	 */
	@Override
	public synchronized boolean doPut(String deviceId, String contentId,
			String content) {

		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to PUT data");
			return false;
		}

		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/devices/" + deviceId
							+ "/measurement/add" + contentId);

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).put(Entity.json(content));

			logger.debug("Received response status to PUT data request = {}",
					Integer.valueOf(response.getStatus()));

			if (response.getStatus() == Response.Status.OK.getStatusCode()) {
				client.close();
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				logger.debug("Sending PUT message resulted in response status UNAUTHORIZED. Trying to login and then again");
				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.put(Entity.json(content));

					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.OK
							.getStatusCode()) {
						client.close();
						return true;
					}

				}
			}

			client.close();

		} catch (Exception e) {
			logger.error("Error in sending PUT message", e);
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doPostAcc(java.lang.
	 * String, java.lang.String)
	 */
	@Override
	public synchronized boolean doPostAcc(String deviceId, String content) {

		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to send data");
			return false;
		}
		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/devices/" + deviceId
							+ "/activity/add");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(content));
			// logger.debug("response status = " + response.getStatus());

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				logger.debug("Sending accelerometer data resulted in response status UNAUTHORIZED. Trying to login and then again");
				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(content));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						return true;
					}

				}
			}

			client.close();

			logger.debug("Sending accelerometer data was not sucessfull");
		} catch (Exception e) {
			logger.error("Error in sending accelerometer data", e);
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doPostPress(java.lang
	 * .String, java.lang.String)
	 */
	@Override
	public boolean doPostPress(String deviceId, String content) {

		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to send data");
			return false;
		}
		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/devices/" + deviceId
							+ "/furniture/add");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(content));
			// logger.debug("response status = " + response.getStatus());

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				logger.debug("Sending furniture pressure data resulted with response status UNAUTHORIZED. Trying to login and then again");

				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(content));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						return true;
					}

				}
			}

			client.close();
			logger.debug("Sending furniture pressure data was not sucessfull");
		} catch (Exception e) {
			logger.error("Error in sending furniture pressure data", e);
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doPostVisual(java.lang
	 * .String, java.lang.String)
	 */
	@Override
	public boolean doPostVisual(String deviceId, String content) {
		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to send data");
			return false;
		}
		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/devices/" + deviceId
							+ "/visual/add");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(content));

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				logger.debug("Sending visual data resulted in response status UNAUTHORIZED. Trying to login and then again");

				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(content));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						return true;
					}

				}
			}

			client.close();
			logger.debug("Sending visual data waas not sucessfull");
		} catch (Exception e) {
			logger.error("Error in sending visual data", e);
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doPostVitlas(java.lang
	 * .String, java.lang.String)
	 */
	@Override
	public boolean doPostVitals(String deviceId, String content) {
		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to send data");
			return false;
		}
		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/devices/" + deviceId
							+ "/vitals/add");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(content));

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				logger.debug("Sending vitals data resulted in response status UNAUTHORIZED. Trying to login and then again");

				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(content));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						return true;
					}

				}
			}

			client.close();
			logger.debug("Sending vitals data was not sucessfull");
		} catch (Exception e) {
			logger.error("Error in sending vitals data", e.toString());
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doPostSpeakerData(java
	 * .lang.String, java.lang.String)
	 */
	@Override
	public boolean doPostSpeakerData(String deviceId, String content) {
		// logger.debug("doPostSpeakerData() called with parameter deviceId = "
		// + deviceId + " and content size = " + content.length());
		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to send data");
			return false;
		}
		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/devices/" + deviceId
							+ "/speaker/add");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(content));
			// logger.debug("response status = " + response.getStatus());

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				logger.debug("Sending speaker data resulted in response status UNAUTHORIZED. Trying to login and then again");

				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(content));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						return true;
					}

				}
			}

			client.close();
			logger.debug("Sending speaker data wasn not sucessfull");
		} catch (Exception e) {
			logger.error("Error in sending speaker data", e.toString());
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doPostAppliancePower
	 * (java.lang.String, java.lang.String)
	 */
	@Override
	public boolean doPostAppliancePower(String deviceId, String content) {
		// logger.debug("doPostAppliancePower() called with parameter deviceId = "
		// + deviceId + " and content size = " + content.length());
		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to send data");
			return false;
		}
		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/devices/" + deviceId
							+ "/power/add");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(content));

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				logger.debug("Sending power sensing data resulted in response status UNAUTHORIZED. Trying to login and then again");
				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(content));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						return true;
					}

				}
			}

			client.close();
			logger.debug("Sending power sensing data was not sucessfull");
		} catch (Exception e) {
			logger.error("Error in sending power sensing data", e);
			return false;
		}
		return false;

	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.comm.RPCloudClient#login()
	 */
	@Override
	public boolean login() {

		logger.debug("Login with username {}", username);

		if (username == null || password == null) {
			logger.debug("Username and/or password are null. Aborting authencication");
			return false;
		}

		try {
			RestTemplate restTemplate = new RestTemplate();
			ResponseEntity<String> response = restTemplate
					.postForEntity(
							this.eWallSystemRoot
									+ "ewall-platform-login/v1/users/authenticate/?username="
									+ username + "&password=" + password, null,
							String.class);

			if (response.getStatusCode() == HttpStatus.OK) {
				JsonElement jelement = new JsonParser().parse(response
						.getBody());
				JsonObject jobject = jelement.getAsJsonObject();
				JsonElement tokenElement = jobject.get("token");
				if (tokenElement != null) {
					this.authToken = tokenElement.getAsString();
					// logger.debug("Login was successful");
					return true;
				}
			}
			logger.warn("Login was not sucessfull. Response status =  {}",
					response.getStatusCode());

		} catch (HttpClientErrorException e) {
			if (e.getStatusCode() == HttpStatus.UNAUTHORIZED)
				logger.warn("Incorrect username or password!");
			logger.error("Error while login", e);
		} catch (ResourceAccessException e) {
			logger.error("Cannot connect to server!", e);
		} catch (JsonSyntaxException e) {
			logger.error("Error in parsing response from login", e);
		} catch (HttpServerErrorException e) {
			logger.error("HTTP Server error while login.", e);
		}

		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#login(java.lang.String,
	 * java.lang.String)
	 */
	@Override
	public boolean login(String username, String password) {
		this.username = username;
		this.password = password;
		return login();
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see eu.ewall.platform.remoteproxy.comm.RPCloudClient#getAuthToken()
	 */
	@Override
	public String getAuthToken() {
		return this.authToken;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doPostNotification(eu
	 * .ewall.platform.commons.datamodel.message.Notification)
	 */
	@Override
	public boolean doPostNotification(Notification notification) {
		if (notification == null) {
			logger.warn("Trying to send notification with null input");
			return false;
		}

		logger.debug("Sensing notification with content {}",
				notification.getContent());
		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to send notification");
			return false;
		}
		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/notifications");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(notification));
			logger.debug("Received response status = {}",
					Integer.valueOf(response.getStatus()));

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(notification));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						return true;
					}

				}
			}

			client.close();

		} catch (Exception e) {
			logger.error(e.toString());
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doPostCaregiverNotification
	 * (eu.ewall.platform.commons.datamodel.message.Notification)
	 */
	@Override
	public boolean doPostCaregiverNotification(Notification notification) {
		logger.debug("doPostCaregiverNotification() called with notification = "
				+ notification.getContent());
		if (!isRegistered) {
			logger.debug("Remote Proxy is not registered at eWALL Cloud. Unable to send notification");
			return false;
		}
		try {
			Client client = ClientBuilder.newClient();

			WebTarget webTarget = client.target(eWallSystemRoot);

			WebTarget resourceWebTarget = webTarget
					.path("cloud-gateway/sensingenvironments/"
							+ sensingEnvironmentId + "/caregiverNotifications");

			Invocation.Builder invocationBuilder = resourceWebTarget.request();

			Response response = invocationBuilder.header("X-Auth-Token",
					this.authToken).post(Entity.json(notification));
			logger.debug("Received response status = {}",
					Integer.valueOf(response.getStatus()));

			if (response.getStatus() == Response.Status.CREATED.getStatusCode()) {
				return true;
			} else if (response.getStatus() == Response.Status.UNAUTHORIZED
					.getStatusCode()) {
				// trying to login again
				if (login()) {
					// if successful
					response = invocationBuilder.headers(null)
							.header("X-Auth-Token", this.authToken)
							.post(Entity.json(notification));
					logger.debug("Received response status = {}",
							Integer.valueOf(response.getStatus()));

					if (response.getStatus() == Response.Status.CREATED
							.getStatusCode()) {
						return true;
					}

				}
			}

			client.close();

		} catch (Exception e) {
			logger.error(e.toString());
			return false;
		}
		return false;
	}

	/*
	 * (non-Javadoc)
	 * 
	 * @see
	 * eu.ewall.platform.remoteproxy.comm.RPCloudClient#doGetLastTimestamp(java
	 * .lang.String, java.lang.String)
	 */
	@Override
	public long doGetLastTimestamp(String deviceId, DeviceType dbType) {
		logger.debug("Getting last timestamp for device type {}", dbType);

		String endpointDbPart = "";

		switch (dbType) {
		case ENVIRONMENTAL_SENSOR:
			endpointDbPart = "environmental";
			break;
		case ACTIVITY_SENSOR:
			endpointDbPart = "activity";
			break;
		case FURNITURE_SENSOR:
			endpointDbPart = "furniture";
			break;
		case VISUAL_SENSOR:
			endpointDbPart = "visual";
			break;
		case VITALS_SENSOR:
			endpointDbPart = "vitals";
			break;
		case AUDIO_SENSOR:
			endpointDbPart = "speaker";
			break;
		case POWER_SENSOR:
			endpointDbPart = "power";
			break;
		default:
			logger.warn("No matching cloud db found for type {}", dbType);
			return Long.MAX_VALUE;
		}

		try {
			HttpHeaders headers = new HttpHeaders();
			headers.set("X-Auth-Token", this.authToken);

			HttpEntity<Void> entity = new HttpEntity<Void>(null, headers);

			RestTemplate restTemplate = new RestTemplate();

			String path = eWallSystemRoot
					+ "cloud-gateway/sensingenvironments/"
					+ sensingEnvironmentId + "/devices/" + deviceId + "/"
					+ endpointDbPart + "/lastTimestamp";

			ResponseEntity<Long> responseBody = restTemplate.exchange(path,
					HttpMethod.GET, entity,
					new ParameterizedTypeReference<Long>() {
					});

			long timestamp = responseBody.getBody().longValue();
			return timestamp;

		} catch (Exception e) {
			logger.error("Error in getting last timestamp", e);
			return Long.MAX_VALUE;
		}
	}

	@Override
	public synchronized SystemPreferences getUserSystemPreferences() {
		logger.debug(
				"Getting user system preferences for senssing environment with id {}",
				sensingEnvironmentId);

		try {
			HttpHeaders headers = new HttpHeaders();
			headers.set("X-Auth-Token", this.authToken);

			HttpEntity<Void> entity = new HttpEntity<Void>(null, headers);

			RestTemplate restTemplate = new RestTemplate();

			String path = eWallSystemRoot
					+ "cloud-gateway/sensingenvironments/"
					+ sensingEnvironmentId + "/systempreferences";

			ResponseEntity<SystemPreferences> response = restTemplate.exchange(
					path, HttpMethod.GET, entity,
					new ParameterizedTypeReference<SystemPreferences>() {
					});
			logger.warn("Response status =  {}", response.getStatusCode());
			if (response.getStatusCode().is2xxSuccessful()) {
				return response.getBody();
			}
		} catch (Exception e) {
			logger.error("Error in obtaing user system preferences", e);
		}
		logger.warn("Unable to retrieve user system preferences.");
		return null;

	}
}
