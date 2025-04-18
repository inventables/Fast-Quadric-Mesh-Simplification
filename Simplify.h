/////////////////////////////////////////////
//
// Mesh Simplification Tutorial
//
// (C) by Sven Forstmann in 2014
//
// License : MIT
// http://opensource.org/licenses/MIT
// https://github.com/sp4cerat/Fast-Quadric-Mesh-Simplification
//

#include <sstream>
#include <float.h> //FLT_EPSILON, DBL_EPSILON

#define loopi(start_l,end_l) for ( int i=start_l;i<end_l;++i )
#define loopi(start_l,end_l) for ( int i=start_l;i<end_l;++i )
#define loopj(start_l,end_l) for ( int j=start_l;j<end_l;++j )
#define loopk(start_l,end_l) for ( int k=start_l;k<end_l;++k )

/* Error Codes for debugging WASM boundary */
enum ErrorCode {
  BadVert = 1,
  InvalidInFormat = 2,
  LoadError = 3,
  InvalidMesh = 4,
  InvalidFraction = 5,
  InvalidTargetCount = 6,
  LargerMesh = 7,
  InvalidOutFormat = 8,
  WriteError = 9
};

/*
 *  Represents an optionally-indexed vertex in space
 */
struct VertexSTL
{
    VertexSTL() {}
    VertexSTL(float x, float y, float z) : x(x), y(y), z(z) {}

    float x, y, z;
    unsigned int i;

    bool operator!=(const VertexSTL& rhs) const
    {
        return x != rhs.x || y != rhs.y || z != rhs.z;
    }
    bool operator<(const VertexSTL& rhs) const
    {
        if      (x != rhs.x)    return x < rhs.x;
        else if (y != rhs.y)    return y < rhs.y;
        else if (z != rhs.z)    return z < rhs.z;
        else                    return false;
    }
};
inline std::string trim(std::string& str)
{
    str.erase(0, str.find_first_not_of(' '));       //prefixing spaces
    str.erase(str.find_last_not_of(' ')+1);         //surfixing spaces
    return str;
}

inline VertexSTL get_vector(std::string& str)
{
    // "vertex float float float"
    float x, y, z;

    if(sscanf(str.c_str(),"vertex %f %f %f",
        &x,	&y,	&z) != 3) {
        // TODO Different exit code to signal this error?
        // printf("weird format ascii stl exiting\n");
        exit(BadVert);
    }
    VertexSTL v(x, y, z);

    // int space0 = str.find_first_of(' ');
    // str.erase(0, space0); // remove "vertex"
    // str.erase(0, str.find_first_not_of(' ')); //prefixing spaces
    // int space1 = str.find_first_of(' ');

    // float x = std::stof(str.substr(0, space1));

    // str.erase(0, space1+1); // remove x
    // str.erase(0, str.find_first_not_of(' ')); //prefixing spaces

    // int space2 = str.find_last_of(' ');

    // VertexSTL v(x,
        // std::stof(str.substr(0, space2)),
        // std::stof(str.substr(space2, str.size()))
    // );
    return v;
}


struct vector3
{
double x, y, z;
};

struct vec3f
{
    double x, y, z;

    inline vec3f( void ) {}

    //inline vec3f operator =( vector3 a )
	// { vec3f b ; b.x = a.x; b.y = a.y; b.z = a.z; return b;}

    inline vec3f( vector3 a )
	 { x = a.x; y = a.y; z = a.z; }

    inline vec3f( const double X, const double Y, const double Z )
    { x = X; y = Y; z = Z; }

    inline vec3f operator + ( const vec3f& a ) const
    { return vec3f( x + a.x, y + a.y, z + a.z ); }

	inline vec3f operator += ( const vec3f& a ) const
    { return vec3f( x + a.x, y + a.y, z + a.z ); }

    inline vec3f operator * ( const double a ) const
    { return vec3f( x * a, y * a, z * a ); }

    inline vec3f operator * ( const vec3f a ) const
    { return vec3f( x * a.x, y * a.y, z * a.z ); }

    inline vec3f v3 () const
    { return vec3f( x , y, z ); }

    inline vec3f operator = ( const vector3 a )
    { x=a.x;y=a.y;z=a.z;return *this; }

    inline vec3f operator = ( const vec3f a )
    { x=a.x;y=a.y;z=a.z;return *this; }

    inline vec3f operator / ( const vec3f a ) const
    { return vec3f( x / a.x, y / a.y, z / a.z ); }

    inline vec3f operator - ( const vec3f& a ) const
    { return vec3f( x - a.x, y - a.y, z - a.z ); }

    inline vec3f operator / ( const double a ) const
    { return vec3f( x / a, y / a, z / a ); }

    inline double dot( const vec3f& a ) const
    { return a.x*x + a.y*y + a.z*z; }

    inline vec3f cross( const vec3f& a , const vec3f& b )
    {
		x = a.y * b.z - a.z * b.y;
		y = a.z * b.x - a.x * b.z;
		z = a.x * b.y - a.y * b.x;
		return *this;
	}

    inline double angle( const vec3f& v )
    {
		vec3f a = v , b = *this;
		double dot = v.x*x + v.y*y + v.z*z;
		double len = a.length() * b.length();
		if(len==0)len=0.00001f;
		double input = dot  / len;
		if (input<-1) input=-1;
		if (input>1) input=1;
		return (double) acos ( input );
	}

    inline double angle2( const vec3f& v , const vec3f& w )
    {
		vec3f a = v , b= *this;
		double dot = a.x*b.x + a.y*b.y + a.z*b.z;
		double len = a.length() * b.length();
		if(len==0)len=1;

		vec3f plane; plane.cross( b,w );

		if ( plane.x * a.x + plane.y * a.y + plane.z * a.z > 0 )
			return (double) -acos ( dot  / len );

		return (double) acos ( dot  / len );
	}

    inline vec3f rot_x( double a )
    {
		double yy = cos ( a ) * y + sin ( a ) * z;
		double zz = cos ( a ) * z - sin ( a ) * y;
		y = yy; z = zz;
		return *this;
	}
    inline vec3f rot_y( double a )
    {
		double xx = cos ( -a ) * x + sin ( -a ) * z;
		double zz = cos ( -a ) * z - sin ( -a ) * x;
		x = xx; z = zz;
		return *this;
	}
    inline void clamp( double min, double max )
    {
		if (x<min) x=min;
		if (y<min) y=min;
		if (z<min) z=min;
		if (x>max) x=max;
		if (y>max) y=max;
		if (z>max) z=max;
	}
    inline vec3f rot_z( double a )
    {
		double yy = cos ( a ) * y + sin ( a ) * x;
		double xx = cos ( a ) * x - sin ( a ) * y;
		y = yy; x = xx;
		return *this;
	}
    inline vec3f invert()
	{
		x=-x;y=-y;z=-z;return *this;
	}
    inline vec3f frac()
	{
		return vec3f(
			x-double(int(x)),
			y-double(int(y)),
			z-double(int(z))
			);
	}

    inline vec3f integer()
	{
		return vec3f(
			double(int(x)),
			double(int(y)),
			double(int(z))
			);
	}

    inline double length() const
    {
		return (double)sqrt(x*x + y*y + z*z);
	}

    inline vec3f normalize( double desired_length = 1 )
    {
		double square = sqrt(x*x + y*y + z*z);
		/*
		if (square <= 0.00001f )
		{
			x=1;y=0;z=0;
			return *this;
		}*/
		//double len = desired_length / square;
		x/=square;y/=square;z/=square;

		return *this;
	}
    static vec3f normalize( vec3f a );

	static void random_init();
	static double random_double();
	static vec3f random();

	static int random_number;

	double random_double_01(double a){
		double rnf=a*14.434252+a*364.2343+a*4213.45352+a*2341.43255+a*254341.43535+a*223454341.3523534245+23453.423412;
		int rni=((int)rnf)%100000;
		return double(rni)/(100000.0f-1.0f);
	}

	vec3f random01_fxyz(){
		x=(double)random_double_01(x);
		y=(double)random_double_01(y);
		z=(double)random_double_01(z);
		return *this;
	}
};


double min(double v1, double v2) {
	return fmin(v1,v2);
}


class SymetricMatrix {

	public:

	// Constructor

	SymetricMatrix(double c=0) { loopi(0,10) m[i] = c;  }

	SymetricMatrix(	double m11, double m12, double m13, double m14,
			            double m22, double m23, double m24,
			                        double m33, double m34,
			                                    double m44) {
			 m[0] = m11;  m[1] = m12;  m[2] = m13;  m[3] = m14;
			              m[4] = m22;  m[5] = m23;  m[6] = m24;
			                           m[7] = m33;  m[8] = m34;
			                                        m[9] = m44;
	}

	// Make plane

	SymetricMatrix(double a,double b,double c,double d)
	{
		m[0] = a*a;  m[1] = a*b;  m[2] = a*c;  m[3] = a*d;
		             m[4] = b*b;  m[5] = b*c;  m[6] = b*d;
		                          m[7 ] =c*c; m[8 ] = c*d;
		                                       m[9 ] = d*d;
	}

	double operator[](int c) const { return m[c]; }

	// Determinant

	double det(	int a11, int a12, int a13,
				int a21, int a22, int a23,
				int a31, int a32, int a33)
	{
		double det =  m[a11]*m[a22]*m[a33] + m[a13]*m[a21]*m[a32] + m[a12]*m[a23]*m[a31]
					- m[a13]*m[a22]*m[a31] - m[a11]*m[a23]*m[a32]- m[a12]*m[a21]*m[a33];
		return det;
	}

	const SymetricMatrix operator+(const SymetricMatrix& n) const
	{
		return SymetricMatrix( m[0]+n[0],   m[1]+n[1],   m[2]+n[2],   m[3]+n[3],
						                    m[4]+n[4],   m[5]+n[5],   m[6]+n[6],
						                                 m[ 7]+n[ 7], m[ 8]+n[8 ],
						                                              m[ 9]+n[9 ]);
	}

	SymetricMatrix& operator+=(const SymetricMatrix& n)
	{
		 m[0]+=n[0];   m[1]+=n[1];   m[2]+=n[2];   m[3]+=n[3];
		 m[4]+=n[4];   m[5]+=n[5];   m[6]+=n[6];   m[7]+=n[7];
		 m[8]+=n[8];   m[9]+=n[9];
		return *this;
	}

	double m[10];
};
///////////////////////////////////////////

namespace Simplify
{
	// Global Variables & Strctures

	struct Triangle { int v[3];double err[4];int deleted,dirty;vec3f n; };
	struct Vertex { vec3f p;int tstart,tcount;SymetricMatrix q;int border;};
	struct Ref { int tid,tvertex; };
	std::vector<Triangle> triangles;
	std::vector<Vertex> vertices;
	std::vector<Ref> refs;

	// Helper functions

	double vertex_error(SymetricMatrix q, double x, double y, double z);
	double calculate_error(int id_v1, int id_v2, vec3f &p_result);
	bool flipped(vec3f p,int i0,int i1,Vertex &v0,Vertex &v1,std::vector<int> &deleted);
	void update_triangles(int i0,Vertex &v,std::vector<int> &deleted,int &deleted_triangles);
	void update_mesh(int iteration);
	void compact_mesh();
	//
	// Main simplification function
	//
	// target_count  : target nr. of triangles
	// agressiveness : sharpness to increase the threashold.
	//                 5..8 are good numbers
	//                 more iterations yield higher quality
	//

	void simplify_mesh(int target_count, double agressiveness=7)
	{
		// init
		loopi(0,triangles.size()) triangles[i].deleted=0;

		// main iteration loop
		int deleted_triangles=0;
		std::vector<int> deleted0,deleted1;
		int triangle_count=triangles.size();
		//int iteration = 0;
		//loop(iteration,0,100)
		for (int iteration = 0; iteration < 100; iteration ++)
		{
			if(triangle_count-deleted_triangles<=target_count)break;

			// update mesh once in a while
			if(iteration%5==0)
			{
				update_mesh(iteration);
			}

			// clear dirty flag
			loopi(0,triangles.size()) triangles[i].dirty=0;

			//
			// All triangles with edges below the threshold will be removed
			//
			// The following numbers works well for most models.
			// If it does not, try to adjust the 3 parameters
			//
			double threshold = 0.000000001*pow(double(iteration+3),agressiveness);

			// remove vertices & mark deleted triangles
			loopi(0,triangles.size())
			{
				Triangle &t=triangles[i];
				if(t.err[3]>threshold) continue;
				if(t.deleted) continue;
				if(t.dirty) continue;

				loopj(0,3)if(t.err[j]<threshold)
				{

					int i0=t.v[ j     ]; Vertex &v0 = vertices[i0];
					int i1=t.v[(j+1)%3]; Vertex &v1 = vertices[i1];
					// Border check
					if(v0.border != v1.border)  continue;

					// Compute vertex to collapse to
					vec3f p;
					calculate_error(i0,i1,p);
					deleted0.resize(v0.tcount); // normals temporarily
					deleted1.resize(v1.tcount); // normals temporarily
					// dont remove if flipped
					if( flipped(p,i0,i1,v0,v1,deleted0) ) continue;

					if( flipped(p,i1,i0,v1,v0,deleted1) ) continue;


					// not flipped, so remove edge
					v0.p=p;
					v0.q=v1.q+v0.q;
					int tstart=refs.size();

					update_triangles(i0,v0,deleted0,deleted_triangles);
					update_triangles(i0,v1,deleted1,deleted_triangles);

					int tcount=refs.size()-tstart;

					if(tcount<=v0.tcount)
					{
						// save ram
						if(tcount)memcpy(&refs[v0.tstart],&refs[tstart],tcount*sizeof(Ref));
					}
					else
						// append
						v0.tstart=tstart;

					v0.tcount=tcount;
					break;
				}
				// done?
				if(triangle_count-deleted_triangles<=target_count)break;
			}
		}
		// clean up mesh
		compact_mesh();
	} //simplify_mesh()

	void simplify_mesh_lossless()
	{
		// init
		loopi(0,triangles.size()) triangles[i].deleted=0;

		// main iteration loop
		int deleted_triangles=0;
		std::vector<int> deleted0,deleted1;
		int triangle_count=triangles.size();
		//int iteration = 0;
		//loop(iteration,0,100)
		for (int iteration = 0; iteration < 9999; iteration ++)
		{
			// update mesh constantly
			update_mesh(iteration);
			// clear dirty flag
			loopi(0,triangles.size()) triangles[i].dirty=0;
			//
			// All triangles with edges below the threshold will be removed
			//
			// The following numbers works well for most models.
			// If it does not, try to adjust the 3 parameters
			//
			double threshold = DBL_EPSILON; //1.0E-3 EPS;

			// remove vertices & mark deleted triangles
			loopi(0,triangles.size())
			{
				Triangle &t=triangles[i];
				if(t.err[3]>threshold) continue;
				if(t.deleted) continue;
				if(t.dirty) continue;

				loopj(0,3)if(t.err[j]<threshold)
				{
					int i0=t.v[ j     ]; Vertex &v0 = vertices[i0];
					int i1=t.v[(j+1)%3]; Vertex &v1 = vertices[i1];

					// Border check
					if(v0.border != v1.border)  continue;

					// Compute vertex to collapse to
					vec3f p;
					calculate_error(i0,i1,p);

					deleted0.resize(v0.tcount); // normals temporarily
					deleted1.resize(v1.tcount); // normals temporarily

					// dont remove if flipped
					if( flipped(p,i0,i1,v0,v1,deleted0) ) continue;
					if( flipped(p,i1,i0,v1,v0,deleted1) ) continue;

					// not flipped, so remove edge
					v0.p=p;
					v0.q=v1.q+v0.q;
					int tstart=refs.size();

					update_triangles(i0,v0,deleted0,deleted_triangles);
					update_triangles(i0,v1,deleted1,deleted_triangles);

					int tcount=refs.size()-tstart;

					if(tcount<=v0.tcount)
					{
						// save ram
						if(tcount)memcpy(&refs[v0.tstart],&refs[tstart],tcount*sizeof(Ref));
					}
					else
						// append
						v0.tstart=tstart;

					v0.tcount=tcount;
					break;
				}
			}
			if(deleted_triangles<=0)break;
			deleted_triangles=0;
		} //for each iteration
		// clean up mesh
		compact_mesh();
	} //simplify_mesh_lossless()


	// Check if a triangle flips when this edge is removed

	bool flipped(vec3f p,int i0,int i1,Vertex &v0,Vertex &v1,std::vector<int> &deleted)
	{

		loopk(0,v0.tcount)
		{
			Triangle &t=triangles[refs[v0.tstart+k].tid];
			if(t.deleted)continue;

			int s=refs[v0.tstart+k].tvertex;
			int id1=t.v[(s+1)%3];
			int id2=t.v[(s+2)%3];

			if(id1==i1 || id2==i1) // delete ?
			{

				deleted[k]=1;
				continue;
			}
			vec3f d1 = vertices[id1].p-p; d1.normalize();
			vec3f d2 = vertices[id2].p-p; d2.normalize();
			if(fabs(d1.dot(d2))>0.999) return true;
			vec3f n;
			n.cross(d1,d2);
			n.normalize();
			deleted[k]=0;
			if(n.dot(t.n)<0.2) return true;
		}
		return false;
	}

	// Update triangle connections and edge error after a edge is collapsed

	void update_triangles(int i0,Vertex &v,std::vector<int> &deleted,int &deleted_triangles)
	{
		vec3f p;
		loopk(0,v.tcount)
		{
			Ref &r=refs[v.tstart+k];
			Triangle &t=triangles[r.tid];
			if(t.deleted)continue;
			if(deleted[k])
			{
				t.deleted=1;
				deleted_triangles++;
				continue;
			}
			t.v[r.tvertex]=i0;
			t.dirty=1;
			t.err[0]=calculate_error(t.v[0],t.v[1],p);
			t.err[1]=calculate_error(t.v[1],t.v[2],p);
			t.err[2]=calculate_error(t.v[2],t.v[0],p);
			t.err[3]=min(t.err[0],min(t.err[1],t.err[2]));
			refs.push_back(r);
		}
	}

	// compact triangles, compute edge error and build reference list

	void update_mesh(int iteration)
	{
		if(iteration>0) // compact triangles
		{
			int dst=0;
			loopi(0,triangles.size())
			if(!triangles[i].deleted)
			{
				triangles[dst++]=triangles[i];
			}
			triangles.resize(dst);
		}
		//
		// Init Quadrics by Plane & Edge Errors
		//
		// required at the beginning ( iteration == 0 )
		// recomputing during the simplification is not required,
		// but mostly improves the result for closed meshes
		//

		// Init Reference ID list
		loopi(0,vertices.size())
		{
			vertices[i].tstart=0;
			vertices[i].tcount=0;
		}
		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			loopj(0,3) vertices[t.v[j]].tcount++;
		}
		int tstart=0;
		loopi(0,vertices.size())
		{
			Vertex &v=vertices[i];
			v.tstart=tstart;
			tstart+=v.tcount;
			v.tcount=0;
		}

		// Write References
		refs.resize(triangles.size()*3);
		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			loopj(0,3)
			{
				Vertex &v=vertices[t.v[j]];
				refs[v.tstart+v.tcount].tid=i;
				refs[v.tstart+v.tcount].tvertex=j;
				v.tcount++;
			}
		}

		if( iteration != 0 ) return;
		//Initial setup: find border vertices and estimate cost function

		// Identify boundary : vertices[].border=0,1

		std::vector<int> vcount,vids;

		loopi(0,vertices.size())
			vertices[i].border=0;

		loopi(0,vertices.size())
		{
			Vertex &v=vertices[i];
			vcount.clear();
			vids.clear();
			loopj(0,v.tcount)
			{
				int k=refs[v.tstart+j].tid;
				Triangle &t=triangles[k];
				loopk(0,3)
				{
					int ofs=0,id=t.v[k];
					while(ofs<vcount.size())
					{
						if(vids[ofs]==id)break;
						ofs++;
					}
					if(ofs==vcount.size())
					{
						vcount.push_back(1);
						vids.push_back(id);
					}
					else
						vcount[ofs]++;
				}
			}
			loopj(0,vcount.size()) if(vcount[j]==1)
				vertices[vids[j]].border=1;
		}

		loopi(0,vertices.size())
		vertices[i].q=SymetricMatrix(0.0);

		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			vec3f n,p[3];
			loopj(0,3) p[j]=vertices[t.v[j]].p;
			n.cross(p[1]-p[0],p[2]-p[0]);
			n.normalize();
			t.n=n;
			loopj(0,3) vertices[t.v[j]].q =
				vertices[t.v[j]].q+SymetricMatrix(n.x,n.y,n.z,-n.dot(p[0]));
		}
		loopi(0,triangles.size())
		{
			// Calc Edge Error
			Triangle &t=triangles[i];vec3f p;
			loopj(0,3) t.err[j]=calculate_error(t.v[j],t.v[(j+1)%3],p);
			t.err[3]=min(t.err[0],min(t.err[1],t.err[2]));
		}


	}

	// Finally compact mesh before exiting

	void compact_mesh()
	{
		int dst=0;
		loopi(0,vertices.size())
		{
			vertices[i].tcount=0;
		}
		loopi(0,triangles.size())
		if(!triangles[i].deleted)
		{
			Triangle &t=triangles[i];
			triangles[dst++]=t;
			loopj(0,3)vertices[t.v[j]].tcount=1;
		}
		triangles.resize(dst);
		dst=0;
		loopi(0,vertices.size())
		if(vertices[i].tcount)
		{
			vertices[i].tstart=dst;
			vertices[dst].p=vertices[i].p;
			dst++;
		}
		loopi(0,triangles.size())
		{
			Triangle &t=triangles[i];
			loopj(0,3)t.v[j]=vertices[t.v[j]].tstart;
		}
		vertices.resize(dst);
	}

	// Error between vertex and Quadric

	double vertex_error(SymetricMatrix q, double x, double y, double z)
	{
 		return   q[0]*x*x + 2*q[1]*x*y + 2*q[2]*x*z + 2*q[3]*x + q[4]*y*y
 		     + 2*q[5]*y*z + 2*q[6]*y + q[7]*z*z + 2*q[8]*z + q[9];
	}

	// Error for one edge

	double calculate_error(int id_v1, int id_v2, vec3f &p_result)
	{
		// compute interpolated vertex

		SymetricMatrix q = vertices[id_v1].q + vertices[id_v2].q;
		bool   border = vertices[id_v1].border & vertices[id_v2].border;
		double error=0;
		double det = q.det(0, 1, 2, 1, 4, 5, 2, 5, 7);
		if ( det != 0 && !border )
		{

			// q_delta is invertible
			p_result.x = -1/det*(q.det(1, 2, 3, 4, 5, 6, 5, 7 , 8));	// vx = A41/det(q_delta)
			p_result.y =  1/det*(q.det(0, 2, 3, 1, 5, 6, 2, 7 , 8));	// vy = A42/det(q_delta)
			p_result.z = -1/det*(q.det(0, 1, 3, 1, 4, 6, 2, 5,  8));	// vz = A43/det(q_delta)

			error = vertex_error(q, p_result.x, p_result.y, p_result.z);
		}
		else
		{
			// det = 0 -> try to find best result
			vec3f p1=vertices[id_v1].p;
			vec3f p2=vertices[id_v2].p;
			vec3f p3=(p1+p2)/2;
			double error1 = vertex_error(q, p1.x,p1.y,p1.z);
			double error2 = vertex_error(q, p2.x,p2.y,p2.z);
			double error3 = vertex_error(q, p3.x,p3.y,p3.z);
			error = min(error1, min(error2, error3));
			if (error1 == error) p_result=p1;
			if (error2 == error) p_result=p2;
			if (error3 == error) p_result=p3;
		}
		return error;
	}

	// Load OBJ from memory buffer
	bool load_obj(const uint8_t* buffer, size_t size) {
		vertices.clear();
		triangles.clear();

		// Ensure valid buffer
		if (!buffer || size == 0) {
			return false;
		}

		std::string data(reinterpret_cast<const char*>(buffer), size);
		std::istringstream stream(data);
		std::string line;
		int vertex_cnt = 0;

		while (std::getline(stream, line)) {
			// Process vertex
			if (line[0] == 'v' && line[1] == ' ') {
				Vertex v;
				std::istringstream lineStream(line.substr(2));
				if (lineStream >> v.p.x >> v.p.y >> v.p.z) {
					vertices.push_back(v);
				}
			}
			// Process face
			else if (line[0] == 'f') {
				std::istringstream lineStream(line.substr(2));
				std::string v1, v2, v3;
				if (lineStream >> v1 >> v2 >> v3) {
					Triangle t;
					// Extract vertex indices (handle different formats of face definitions)
					size_t slashPos = v1.find('/');
					if (slashPos != std::string::npos) {
						v1 = v1.substr(0, slashPos);
					}
					slashPos = v2.find('/');
					if (slashPos != std::string::npos) {
						v2 = v2.substr(0, slashPos);
					}
					slashPos = v3.find('/');
					if (slashPos != std::string::npos) {
						v3 = v3.substr(0, slashPos);
					}

					// Convert to integers and adjust indices
					t.v[0] = std::stoi(v1) - 1 - vertex_cnt;
					t.v[1] = std::stoi(v2) - 1 - vertex_cnt;
					t.v[2] = std::stoi(v3) - 1 - vertex_cnt;

					triangles.push_back(t);
				}
			}
		}

		return (vertices.size() > 0 && triangles.size() > 0);
	} // load_obj()

	// Serialize mesh as OBJ to memory buffer
	bool write_obj(uint8_t** buffer, size_t* size) {
		std::stringstream ss;

		// Write vertices
		for (size_t i = 0; i < vertices.size(); i++) {
			ss << "v " << vertices[i].p.x << " " << vertices[i].p.y << " " << vertices[i].p.z << "\n";
		}

		// Write faces
		for (size_t i = 0; i < triangles.size(); i++) {
			if (!triangles[i].deleted) {
				ss << "f " << (triangles[i].v[0]+1) << " " << (triangles[i].v[1]+1) << " " << (triangles[i].v[2]+1) << "\n";
			}
		}

		// Get the string and copy to buffer
		std::string str = ss.str();
		*size = str.size();
		*buffer = (uint8_t*)malloc(*size);
		if (!*buffer) {
			return false;
		}

		memcpy(*buffer, str.c_str(), *size);
		return true;
	}

    // Load binary STL from memory buffer
    std::vector<VertexSTL> load_binary_stl(const uint8_t* buffer, size_t size) {
        if (!buffer || size < 84) {
            return std::vector<VertexSTL>();
        }

        // STL header is 80 bytes followed by 4-byte face count
        int num_faces;
        std::memcpy(&num_faces, buffer + 80, 4);

        // Validate buffer size
        size_t required_size = 84 + (num_faces * 50); // Header + count + faces data
        if (size < required_size) {
            return std::vector<VertexSTL>();
        }

        const unsigned int num_indices = num_faces * 3;
        std::vector<VertexSTL> all_vertices(num_indices);

        for (int i = 0; i < num_faces; i++) {
            for (int j = 0; j < 3; j++) {
                const int index = i * 3 + j;
                const int position = 84 + 12 + i * 50 + j * 12; // Skip normal (12 bytes) for each face
                std::memcpy(&all_vertices[index], buffer + position, 12);
            }
        }

        return all_vertices;
    }

    // Load ASCII STL from memory buffer
    std::vector<VertexSTL> load_ascii_stl(const uint8_t* buffer, size_t size) {
        std::vector<VertexSTL> all_vertices;

        if (!buffer || size == 0) {
            return all_vertices;
        }

        std::string data(reinterpret_cast<const char*>(buffer), size);
        std::istringstream stream(data);
        std::string line;

        while (std::getline(stream, line)) {
            line = trim(line);
            if (line.rfind("vertex", 0) == 0) {
                all_vertices.push_back(get_vector(line));
            }
        }

        return all_vertices;
    }

    // Detect STL format (ASCII or binary) and load vertices
    std::vector<VertexSTL> load_stl_vertices(const uint8_t* buffer, size_t size) {
        if (!buffer || size < 6) {
            return std::vector<VertexSTL>();
        }

        // Check if it starts with "solid " (ASCII STL)
        std::string header(reinterpret_cast<const char*>(buffer), 6);
        if (header == "solid ") {
            // Further check for "facet" to confirm it's really ASCII
            std::string data(reinterpret_cast<const char*>(buffer), std::min(size, size_t(256)));
            size_t pos = data.find("facet");
            if (pos != std::string::npos) {
                return load_ascii_stl(buffer, size);
            }
        }

        // Default to binary
        return load_binary_stl(buffer, size);
    }


    // Load STL from memory buffer
    bool load_stl(const uint8_t* buffer, size_t size) {
        vertices.clear();
        triangles.clear();

        if (!buffer || size == 0) {
            return false;
        }

        std::vector<VertexSTL> all_vertices = load_stl_vertices(buffer, size);
        const int32_t num_indices = all_vertices.size();

        if (num_indices == 0) {
            return false;
        }

        for (int c = 0; c < all_vertices.size(); c++) {
            all_vertices[c].i = c;
        }

        int32_t *indices = (int32_t*)malloc(num_indices * sizeof(int32_t));
        if (!indices) {
            return false;
        }

        std::sort(all_vertices.begin(), all_vertices.end());

        float minx =  999999;
        float miny =  999999;
        float minz =  999999;
        float maxx = -999999;
        float maxy = -999999;
        float maxz = -999999;

        unsigned int num_vertices = 0;
        for (int i = 0; i < all_vertices.size(); i++) {
            VertexSTL v = all_vertices[i];
            if (!num_vertices || v != all_vertices[num_vertices-1]) {
                all_vertices[num_vertices++] = v;
                if (v.x < minx) minx = v.x;
                if (v.x > maxx) maxx = v.x;
                if (v.y < miny) miny = v.y;
                if (v.y > maxy) maxy = v.y;
                if (v.z < minz) minz = v.z;
                if (v.z > maxz) maxz = v.z;
            }
            indices[v.i] = num_vertices - 1;
        }
        all_vertices.resize(num_vertices);

        // Add vertices
        for (int i = 0; i < all_vertices.size(); i++) {
            VertexSTL _v = all_vertices[i];
            Vertex v;
            v.p.x = _v.x; v.p.y = _v.y; v.p.z = _v.z;
            vertices.push_back(v);
        }

        // Add triangles
        for (int i = 0; i < num_indices; i += 3) {
            Triangle t;
            t.v[0] = indices[i];
            t.v[1] = indices[i+1];
            t.v[2] = indices[i+2];
            triangles.push_back(t);
        }

        free(indices);
        return true;
    }

    // Helper functions to write binary STL
    inline void write_float_to_buffer(float f, uint8_t* buffer, size_t& offset) {
        memcpy(buffer + offset, &f, sizeof(f));
        offset += sizeof(f);
    }

    inline void write_vertex_stl_to_buffer(vec3f v, uint8_t* buffer, size_t& offset) {
        write_float_to_buffer(v.x, buffer, offset);
        write_float_to_buffer(v.y, buffer, offset);
        write_float_to_buffer(v.z, buffer, offset);
    }

    // Serialize mesh as binary STL to memory buffer
    bool write_stl(uint8_t** buffer, size_t* size) {
        // Count non-deleted triangles
        unsigned int number_triangles = 0;
        for (size_t i = 0; i < triangles.size(); i++) {
            if (!triangles[i].deleted) {
                number_triangles += 1;
            }
        }

        // Calculate buffer size: 80 byte header + 4 byte count + 50 bytes per triangle
        *size = 84 + (number_triangles * 50);
        *buffer = (uint8_t*)malloc(*size);

        if (!*buffer) {
            return false;
        }

        // Initialize the buffer
        memset(*buffer, 0, *size);

        // Write header (80 bytes)
        const char header[] = "SIMPLIFIED MESH";
        memcpy(*buffer, header, std::min(sizeof(header), size_t(80)));

        // Write triangle count (4 bytes)
        memcpy(*buffer + 80, &number_triangles, 4);

        // Write triangle data
        size_t offset = 84;
        for (size_t i = 0; i < triangles.size(); i++) {
            if (!triangles[i].deleted) {
                vec3f v0 = vertices[triangles[i].v[0]].p;
                vec3f v1 = vertices[triangles[i].v[1]].p;
                vec3f v2 = vertices[triangles[i].v[2]].p;

                // Calculate normal
                vec3f n;
                n.cross(v1-v0, v2-v0);
                n.normalize();

                // Write normal and vertices (12 bytes each)
                write_vertex_stl_to_buffer(n, *buffer, offset);
                write_vertex_stl_to_buffer(v0, *buffer, offset);
                write_vertex_stl_to_buffer(v1, *buffer, offset);
                write_vertex_stl_to_buffer(v2, *buffer, offset);

                // Write attribute byte count (2 bytes of zeros)
                offset += 2;
            }
        }

        return true;
    }

};
///////////////////////////////////////////
