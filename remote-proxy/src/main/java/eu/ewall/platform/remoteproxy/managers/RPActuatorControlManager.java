/*******************************************************************************
 * Copyright 2015 Ericsson Nikola Tesla d.d.
 ******************************************************************************/
package eu.ewall.platform.remoteproxy.managers;

import org.slf4j.Logger;
import org.slf4j.LoggerFactory;

import eu.ewall.gateway.lightcontrol.PHHueController;
import eu.ewall.platform.commons.datamodel.ewallsystem.ActuatorCommand;

/**
 * The Class RPActuatorControlManager.
 */
public class RPActuatorControlManager {

	/** The log. */
	private static Logger LOG = LoggerFactory
			.getLogger(RPActuatorControlManager.class);

	/**
	 * Execute command.
	 *
	 * @param command
	 *            the command
	 * @throws Exception
	 *             the exception
	 */
	public void executeCommand(ActuatorCommand command) throws Exception {
		PHHueController hueController = PHHueController.getInstance();
		int commandValue = command.getCommandValue();
		int lightID = getLightIDFromActuatorName(command.getActuatorName());
		
		LOG.debug("Executing actuator command (type = {}, value = {}, light id = {})",command.getCommandType(), Integer.valueOf(commandValue), Integer.valueOf(lightID));

		switch (command.getCommandType()) {
		case SET_STATUS:
			if (commandValue == 1) {
				hueController.onOff(lightID, true);
			} else if (commandValue == 0) {
				hueController.onOff(lightID, false);
			}
			break;
		case SET_INTENSITY:
			hueController.setLightIntensityPercent(lightID, commandValue);
			break;
		case CHANGE_INTENSITY:
			hueController.changeLightIntensityPercent(lightID, commandValue);
			break;
		case SET_COLOR:
			hueController.setLightRGBAll(commandValue);
			break;
		default:
			break;
		}

		LOG.info("Executed command for actuator {} ", command.getActuatorName());
	}

	/**
	 * Gets the light id from actuator name.
	 *
	 * @param actuatorName
	 *            the actuator name
	 * @return the light id from actuator name
	 */
	private int getLightIDFromActuatorName(String actuatorName) {
		int hashSeparatorIndex = actuatorName.lastIndexOf("#");
		try {
			String lightID = actuatorName.substring(hashSeparatorIndex + 1);
			return Integer.parseInt(lightID);

		} catch (Exception e) {
			LOG.debug("Light id not set in actuator name. Returning -1", e);
			return -1;
		}
	}
}
