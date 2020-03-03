void findSunsAltAndAzOne(double year, double month, double day, double timezone, double hour, double minute, double second, double latitude, double longitude){
	longitude = (longitude * -1.0) / 15.0;
	double jd = findJD(year, month, day, timezone, hour, minute, second);
	double t, l, m, c, o;
	double omega, lambda, epsilon, delta, alpha, theta, h;

	// (Meeus Pages 163-164) vvv
	//Time in Julian Centuries
	t = ((jd - 2451545.0) / 36525.0);
	//Mean equinox of the date
	l = 280.46645 + 36000.76983 * t + 0.0003032 * t * t;
	//Mean Anomaly of the Sun 
	m = 357.52910 + 35999.05030 * t - 0.0001559 * t * t - 0.00000048 * t * t * t;

	// Sun's Equation of the center
	c = (1.914600 - 0.004817 * t - 0.000014 * t * t) * sin(toRadians(m)) + (0.019993 - 0.000101 * t) * sin(2 * toRadians(m)) + 0.000290 * sin(3 * toRadians(m));
	//Sun's True Longitude
	o = l + c;

	//Brings 'o' within + or - 360 degrees. (Taking an inverse function of very large numbers can sometimes lead to slight errors in output)
	o = within360(o);

	//###############
	//(Meeus Page 164)
	//Sun's Apparent Longitude (The Output of Lambda)
	omega = 125.04 - 1934.136 * t;
	lambda = o - 0.00569 - 0.00478 * sin(toRadians(omega));
	//Brings 'lambda' within + or - 360 degrees. (Taking an inverse function of very large numbers can sometimes lead to slight errors in output)
	lambda = within360(lambda);

	//###############
	//Obliquity of the Ecliptic (Meeus page 147) (numbers switched from degree minute second in book to decimal degree)
	epsilon = (23.4392966666667 - 0.012777777777777778 * t - 0.00059 / 60.0 * t * t + 0.00059 / 60.0 * t * t * t) + (0.00256 * cos(toRadians(omega)));
	//Sun's Declination (Meeus page 165)
	delta = toDegrees(asin(sin(toRadians(epsilon)) * sin(toRadians(lambda))));
	//Sun's Right Acension (Meeus page 165) (divided by 15 to convert to hours)
	alpha = toDegrees(atan2(((cos(toRadians(epsilon)) * sin(toRadians(lambda)))), (cos(toRadians(lambda))))) / 15.0;
	if (alpha < 0){
		alpha = alpha + 24;
	}

	//Sidereal Time (Meeus Page 88)
	theta = (280.46061837 + 360.98564736629 * (jd - 2451545.0) + 0.000387933 * t * t - ((t * t * t) / 38710000.0)) / 15.0;
	//Brings 'theta' within + or - 360 degrees. (Taking an inverse function of very large numbers can sometimes lead to slight errors in output)
	theta = within360(theta);
	//The Local Hour Angle (Meeus Page 92) (multiplied by 15 to convert to degrees)
	h = (theta - longitude - alpha) * 15.0;
	//Brings 'h' within + or - 360 degrees. (Taking an inverse function of very large numbers can sometimes lead to slight errors in output)
	h = within360(h);
	//############
	//Local Horizontal Coordinates (Meeus Page 93)
	//Altitude
	SunsAltitude = toDegrees(asin(sin(toRadians(latitude)) * sin(toRadians(delta)) + cos(toRadians(latitude)) * cos(toRadians(delta)) * cos(toRadians(h))));
	//Azimuth
	SunsAzimuth = toDegrees(atan2((sin(toRadians(h))), ((cos(toRadians(h)) * sin(toRadians(latitude))) - tan(toRadians(delta)) * cos(toRadians(latitude))))) + useNorthAsZero;

}

double findJD(double year, double month, double day, double timezone, double hour, double minute, double second){
	double m = month;
	double jd;
	double d = day + ((hour + (-1.0 * timezone)) / 24.0) + minute / 1440.0 + second / 86400;
	if (m > 2){}
	else{
		year = year - 1;
		m = m + 12;
	}
	double a = (double)((long)(year / 100.0));
	double b = 2 - a + (double)((long)(a / 4.0));
	jd = floor(365.25 * (year + 4716.0)) + floor(30.6001 * (m + 1)) + d + b + -1524.5;
	return jd;
}

double within360(double angle)
{
	double k = floor(angle / 360.0);
	angle = angle - 360.0 * k;
	return angle;
}

double toRadians(double angle){
	angle = angle * (PI/180);
	return angle; 
}

double toDegrees(double angle){
	angle = angle * (180/PI);
	return angle; 
}
