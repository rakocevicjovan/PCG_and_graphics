//holds SDFs for various useful shapes and helpful transformative operations


/* Helpers, move to separate file eventually when there's a sizable number */

float projectedRatio(float3 toProject, float3 onTo)
{
	return (toProject.Dot(onTo) / onto.Dot(onto));
}

float3 project(float3 toProject, float3 onTo)
{
	return projectedRatio(toProject, onTo) * onTo;
}



/*  SDFs */

float sdTorus(float3 p, float2 t)
{
	return length(float2(length(p.xz) - t.x, p.y)) - t.y;
}


float sdCylinder(float3 p, float3 c)
{
	return length(p.xz - c.xy) - c.z;
}


float sdLineSegment(float3 p, float3 a, float3 b, float lineThickness)
{
	// Project p onto the line in which points a and b lie, to get point q. Clamp this to [a, b], get distance to it
	float3 ap = p - a;
	float3 ab = b - a;
	
	// Find projection of ap onto ab clamped to the line segment, find projected point q 
	float t = clamp(projectedRatio(ap, ab), 0.f, 1.f);

	return length(a + t * ab - p) - lineThickness;

	/* Can be rewritten as:
		float t = min(1., max(0., dot(ap, ab) / dot(ab, ab)));		// Functionally same as the clamp above
		return length(p - a - (b - a) * t) - lineThickness;			// Distance between P in space of A and Q in space of A, so basically the same again
		
		Proof that they are the same. Ignore lineThickness since it's a constant in both, length -> |x| notation
		|p - a - (b - a) * t|	== |a + t * ab - p|					// t * ab = t * (b - a) = tb - ta, substitute
		|p - a - tb + ta|		== |a + tb - ta - p|				// reorder so they are in the same order
		 p - a - tb + ta		=  -1 * (-p + a + tb - ta)			// length doesn't change, QED
	*/
}


/* Transformative operations */

float3 opTwist(float3 p, float angle)
{
	float c = cos(angle);
	float s = sin(angle);
	float2x2 rotoMato = float2x2(c, -s, s, c);
	return float3(mul(p.xz, rotoMato), p.y);
}