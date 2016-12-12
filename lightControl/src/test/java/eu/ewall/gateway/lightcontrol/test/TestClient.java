package eu.ewall.gateway.lightcontrol.test;

import eu.ewall.gateway.lightcontrol.PHHueController;
import eu.ewall.gateway.lightcontrol.PHHueInitializer;
import eu.ewall.gateway.lightcontrol.PHHueKiller;

import java.util.Set;

import eu.ewall.gateway.lightcontrol.ConfigureLightControl;

/**
 * The Class TestClient.
 */
public class TestClient {

	/**
	 * The main method.
	 *
	 * @param args the arguments
	 */
	public static void main(String[] args) {
		try {
			handleLights();
		} catch (Exception e) {
			System.out.println(e.getMessage());
		}
//		Set<Thread> threadSet = Thread.getAllStackTraces().keySet();
//		System.out.println("Threads still running: " + threadSet);
	}

	/**
	 * Handle lights.
	 *
	 * @throws Exception the exception
	 */
	private static void handleLights() throws Exception {

		System.out.println("Trying to initialize");

		PHHueController controller = PHHueController.getInstance();
		ConfigureLightControl.firstRun();
		try {
			Thread.sleep(1000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		
		
		
		System.out.println("Tester function trying to turn light 0 on");

		controller.onOff(1, true);

		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out
				.println("Tester function trying to set random value for all lights");

		controller.randomLights();

		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out
				.println("Tester function trying to set random value for all lights");

		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out.println("Tester function trying to turn light 0 off");

		controller.onOff(1, false);
		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out.println("Tester function trying to turn light 0 on");

		controller.onOff(1, true);

		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out.println("Tester function trying to turn light 0 red");

		controller.setLight(1, PHHueController.rgbToHsb(255, 0, 0));

		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out.println("Tester function trying to turn light 0 green");

		controller.setLight(1, PHHueController.rgbToHsb(0, 255, 0));

		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out.println("Tester function trying to turn light 0 blue");

		controller.setLight(1, PHHueController.rgbToHsb(0, 0, 255));

		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out.println("Tester function trying to turn light 0 off");

		controller.onOff(1, false);

		System.out
				.println("Tester function trying to turn light 5 off (this shouldn't work as we don't have 6 lights)");

		controller.onOff(6, true); // this shouldn't work as we don't have 6
									// lights

		try {
			Thread.sleep(3000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}

		System.out.println("Trying to release resources");

		PHHueKiller.kill();
		
	}
	
}
