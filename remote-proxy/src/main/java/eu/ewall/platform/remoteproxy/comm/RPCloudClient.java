/*******************************************************************************
 * Copyright 2014 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.comm;

import java.util.List;

import eu.ewall.platform.commons.datamodel.device.Device;
import eu.ewall.platform.commons.datamodel.device.DeviceType;
import eu.ewall.platform.commons.datamodel.ewallsystem.PointOfContact;
import eu.ewall.platform.commons.datamodel.message.Notification;
import eu.ewall.platform.commons.datamodel.profile.preferences.SystemPreferences;
import eu.ewall.platform.remoteproxy.api.MethodResult;

/**
 * The Interface RPRestClient.
 *
 * @author emirmos
 */
public interface RPCloudClient {

	/**
	 * Register.
	 *
	 * @param poc
	 *            the poc
	 * @return the method result
	 */
	MethodResult register(PointOfContact poc);

	/**
	 * De-register.
	 *
	 * @param poc
	 *            the poc
	 * @return true, if successful
	 */
	boolean deRegister(PointOfContact poc);

	/**
	 * Do post.
	 *
	 * @param deviceId
	 *            the device id
	 * @param content
	 *            the content
	 * @return true, if successful
	 */
	boolean doPost(String deviceId, String content);

	/**
	 * Do post.
	 *
	 * @param deviceId
	 *            the device id
	 * @param content
	 *            the content
	 * @return true, if successful
	 */
	boolean doPostAcc(String deviceId, String content);

	/**
	 * Do put.
	 *
	 * @param deviceId
	 *            the device id
	 * @param contentId
	 *            the content id
	 * @param content
	 *            the content
	 * @return true, if successful
	 */
	boolean doPut(String deviceId, String contentId, String content);

	/**
	 * Do post press.
	 *
	 * @param deviceId
	 *            the device id
	 * @param content
	 *            the content
	 * @return true, if successful
	 */
	boolean doPostPress(String deviceId, String content);

	/**
	 * Do post visual.
	 *
	 * @param deviceId
	 *            the device id
	 * @param content
	 *            the content
	 * @return true, if successful
	 */
	boolean doPostVisual(String deviceId, String content);

	/**
	 * Do post health.
	 *
	 * @param deviceId
	 *            the device id
	 * @param content
	 *            the content
	 * @return true, if successful
	 */
	boolean doPostVitals(String deviceId, String content);

	/**
	 * Do post speaker data.
	 *
	 * @param deviceId
	 *            the device id
	 * @param content
	 *            the content
	 * @return true, if successful
	 */
	boolean doPostSpeakerData(String deviceId, String content);

	/**
	 * Do post appliance power.
	 *
	 * @param deviceId
	 *            the device id
	 * @param content
	 *            the content
	 * @return true, if successful
	 */
	boolean doPostAppliancePower(String deviceId, String content);

	/**
	 * Do get last timestamp.
	 *
	 * @param deviceId
	 *            the device id
	 * @param dbType
	 *            the db type
	 * @return the long
	 */
	long doGetLastTimestamp(String deviceId, DeviceType dbType);

	/**
	 * Do post notification.
	 *
	 * @param notification
	 *            the notification
	 * @return true, if successful
	 */
	boolean doPostNotification(Notification notification);

	/**
	 * Do post caregiver notification.
	 *
	 * @param notification
	 *            the notification
	 * @return true, if successful
	 */
	boolean doPostCaregiverNotification(Notification notification);

	
	/**
	 * Gets the user system preferences.
	 *
	 * @return the user system preferences
	 */
	SystemPreferences getUserSystemPreferences();
	
	
	/**
	 * Logins to the eWALL system, and stores received security token.
	 *
	 * @param username
	 *            the username
	 * @param password
	 *            the password
	 * @return true, if successful
	 */
	boolean login(String username, String password);

	/**
	 * Logins to the eWALL system, and stores received security token.
	 *
	 * @return true, if successful
	 */
	boolean login();

	/**
	 * Gets the auth token.
	 *
	 * @return the auth token
	 */
	String getAuthToken();

	/**
	 * Gets the device info.
	 *
	 * @return the device info
	 */
	List<Device> getDeviceInfo();

}
