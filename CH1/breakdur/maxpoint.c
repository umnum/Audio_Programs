BREAKPOINT maxpoint(const BREAKPOINT *points, long npoints)
{
	int i;
	BREAKPOINT point; 
	point.time = points[0].time;
	point.value = points[0].value;
	for (i = 0; i < npoints; i++)
	{
		if (point.value < points[i].value)
		{
			point.value = points[i].value; 
			point.time = points[i].time;
		}
	}
	return point;
}
