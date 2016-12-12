package eu.ewall.sensing.socket;

import java.util.List;

import com.fasterxml.jackson.annotation.JsonProperty;

public class CircleInfo {
	@JsonProperty(value = "static")
	private List<Circle> circles;

	public List<Circle> getCircles() {
		return circles;
	}
	public void setCircles(List<Circle> circles) {
		this.circles = circles;
	}
	
}
