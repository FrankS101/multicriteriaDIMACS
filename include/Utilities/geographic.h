#ifndef GEOGRAPHIC_H
#define GEOGRAPHIC_H

#include <cmath>
#include <math.h>

#define pi_value 3.14159265358979323846

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts decimal degrees to radians             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double deg2rad(double deg) {
  return (deg * pi_value / 180);
}

/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
/*::  This function converts radians to decimal degrees             :*/
/*:::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::::*/
double rad2deg(double rad) {
  return (rad * 180 / pi_value);
}

double haversineDistanceInMeters(double lat1, double lon1, double lat2, double lon2) {
  double theta, dist;
  theta = lon1 - lon2;
  dist = sin(deg2rad(lat1)) * sin(deg2rad(lat2)) + cos(deg2rad(lat1)) * cos(deg2rad(lat2)) * cos(deg2rad(theta));
  dist = acos(dist);
  dist = rad2deg(dist);
  dist = dist * 60 * 1.1515;
  dist = dist * 1609.344;
  return (dist);
}

double greatCircle(double lat1, double long1, double lat2, double long2) {
	double PI = 4.0*atan(1.0);
        
   //main code inside the class
    double dlat1=lat1*(PI/180);

    double dlong1=long1*(PI/180);
    double dlat2=lat2*(PI/180);
    double dlong2=long2*(PI/180);

    double dLong=dlong1-dlong2;
    double dLat=dlat1-dlat2;

    double aHarv= pow(sin(dLat/2.0),2.0)+cos(dlat1)*cos(dlat2)*pow(sin(dLong/2),2);
    double cHarv=2*atan2(sqrt(aHarv),sqrt(1.0-aHarv));
    //earth's radius from wikipedia varies between 6,356.750 km — 6,378.135 km (˜3,949.901 — 3,963.189 miles)
    //The IUGG value for the equatorial radius of the Earth is 6378.137 km (3963.19 mile)
    const double earth=3963.19;//I am doing miles, just change this to radius in kilometers to get distances in km
    double distance=earth*cHarv;

	return distance*1000;
}

double euclideanDistance( double x1, double y1, double x2, double y2)
{
    return sqrt(pow(abs( x1 - x2),2) + pow(abs( y1 - y2),2));
}


std::pair<double,double> ToGeographic(std::pair<double,double> mercator)
{
    std::pair<double,double> geographic;
    std::cout << mercator.first << " " << mercator.second << std::endl;
    if (abs(mercator.first) < 180 && abs(mercator.second) < 90)
        return std::pair<double,double>(0,0);

    if ( ( abs(mercator.first) > 20037508.3427892) || (abs(mercator.second) > 20037508.3427892) )
        return std::pair<double,double>(0,0);

    double x = mercator.first;
    double y = mercator.second;
    double num3 = x / 6378137.0;
    double num4 = num3 * 57.295779513082323;
    double num5 = floor((double)((num4 + 180.0) / 360.0));
    double num6 = num4 - (num5 * 360.0);
    double num7 = 1.5707963267948966 - (2.0 * atan(exp((-1.0 * y) / 6378137.0)));
    geographic.first = num6;
    geographic.second = num7 * 57.295779513082323;
    return geographic;
}

std::pair<double,double> ToWebMercator(std::pair<double,double> geographic)
{
    std::pair<double,double> mercator;
    std::cout << abs(geographic.first) << " " << abs(geographic.second) << std::endl;
    if ((abs(geographic.first) > 180 || abs(geographic.second) > 90))
        return std::pair<double,double>(0,0);

    double num = geographic.first * 0.017453292519943295;
    double x = 6378137.0 * num;
    double a = geographic.second * 0.017453292519943295;

    mercator.first = x;
    mercator.second = 3189068.5 * log((1.0 + sin(a)) / (1.0 - sin(a)));
    return mercator;
}

#endif //GEOGRAPHIC_H
