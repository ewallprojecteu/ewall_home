package eu.ewall.platform.local.reasoners;

import java.util.LinkedList;
import java.util.Queue;

public class VitalsSensingDataQueue {
	
	private static VitalsSensingDataQueue instance = new VitalsSensingDataQueue();
	
	private Queue<Integer> queue = new LinkedList<Integer>();
	
	private VitalsSensingDataQueue() {
		
	}
	
	public static VitalsSensingDataQueue getInstance() {
		
		return instance;
	}

	public Queue<Integer> getQueue() {
		return queue;
	}

	public void setQueue(Queue<Integer> queue) {
		this.queue = queue;
	}
	
	public void empty() {
		queue.removeAll(queue);
	}
	
}
