package eu.ewall.gateway.lightcontrol;

import eu.ewall.gateway.lightcontrol.PHHueController;
import eu.ewall.gateway.lightcontrol.PHHueInitializer;
import eu.ewall.gateway.lightcontrol.PHHueKiller;

/**
 * The Class TestClient.
 */
public class ConfigureLightControl {

	/*
	 * @param args the arguments
	 */
	public static void main(String[] args) {
		try {
			firstRun();
		} catch (Exception e) {
			System.out.println(e.getMessage());
		}
		return;
	}
	
	
	/**
	 * Handle lights.
	 *
	 * @throws Exception the exception
	 */
	public static void firstRun() throws Exception {

		PHHueInitializer.initialize();

		try {
			Thread.sleep(1000); // 1000 milliseconds is one second.
		} catch (InterruptedException ex) {
			Thread.currentThread().interrupt();
		}


		PHHueKiller.kill();

	}

}
