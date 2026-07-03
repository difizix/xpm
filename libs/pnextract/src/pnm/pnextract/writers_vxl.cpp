
#include "blockNet.h"

///@cond INTERNAL


voxelField<float> ballRadiiToVoxel(const blockNetwork& mpn)  {
	cout<< " write_radius   "<<endl;
	voxelField<float> vfild(mpn.cg.nx,mpn.cg.ny,mpn.cg.nz,0);
	{
		const medialSurface & rf = *mpn.srf;

		float maxr=0.,  minr=0.;
		for(const auto& vi: rf.vxlSpace)  {
			vfild(vi.i,vi.j,vi.k) = vi.R;
			maxr=max(maxr,vi.R);
			minr=min(minr,vi.R);
		}
		cout<< "  radius  [" << minr <<"  " << maxr <<"]   nVs:" << rf.vxlSpace.size() <<endl;
	}
	return vfild;
}



/*//voxelImageT<int> VElemsPlusThroats(const blockNetwork& mpn)  {
	//cout<< " VElemsPlusThroats   "<<endl;
	//voxelImageT<int> vfild(mpn.VElems);
	//int throatValue=mpn.poreIs.size()+1000000;
	//for(const auto tr: mpn.throatIs)
		//for (const auto  vi: tr->toxels2)
			//vfild(vi->i+1,vi->j+1,vi->k+1) = throatValue;
	//return vfild;
//}*/





///.  Originally written by Tom Bultreys {

voxelField<int> VThroats(const blockNetwork& mpn)  {
	cout<< " write_throats   "<<endl;
	voxelField<int> vfild (mpn.VElems.size3() , 0);
	for(const auto tr: mpn.throatIs)  {
		int throatValue = tr->tid + 1;
		for (const auto  vi: tr->toxels2)
			vfild(vi->i+1,vi->j+1,vi->k+1) = throatValue;
	}
	return vfild;
}

voxelField<int> VThroats(const blockNetwork& mpn, int beginSlice, int endSlice)  {
	cout<< " write_throats   "<<endl;
   int x,y,zdummy;
	mpn.VElems.getSize(x,y,zdummy);
	voxelField<int> vfild (int3(x,y,endSlice - beginSlice) , 0);
	for(const auto tr: mpn.throatIs)  {
		int throatValue = tr->tid + 1;
		for (const auto  vi: tr->toxels2)
			if( vi->k+1 >= beginSlice && vi->k+1 < endSlice)
				vfild(vi->i+1, vi->j+1, vi->k+1-beginSlice) = throatValue;
	}
	return vfild;
}



voxelField<int> poreMaxBalls(const blockNetwork& mpn)  {
	cout<< " write_poreMaxBalls   "<<endl;
	voxelField<int> vfild(mpn.cg.nx, mpn.cg.ny, mpn.cg.nz, 0);

	const medialSurface & rf = *mpn.srf;

	for (size_t ip = mpn.firstPores; ip < mpn.poreIs.size(); ++ip) {
		const poreNE* pr = mpn.poreIs[ip];
		if (pr && pr->mb) {
			int x = pr->mb->fi, y = pr->mb->fj, z = pr->mb->fk;
			float rlim = pr->mb->R * pr->mb->R;
			int ex = 1 * sqrt(rlim);
			int elId = ip;

			for (int a = -ex; a <= ex; ++a)  {
				int ey = sqrt(rlim - a * a);
				for (int b = -ey; b <= ey; ++b)  {
					int ez = sqrt(rlim - a * a - b * b);
					for (int c = -ez; c <= ez; ++c) {
						if (rf.isInside(x + a, y + b, z + c) && vfild(x + a, y + b, z + c) == 0) {
							vfild(x + a, y + b, z + c) = elId;
						}
					}
				}
			}
		}
	}
	return vfild;
}

voxelField<int> poreMaxBalls(const blockNetwork& mpn, int firstSlice, int lastSlice)  {
	cout<< " write_poreMaxBalls   "<<endl;

	voxelField<int> vfild(mpn.cg.nx, mpn.cg.ny, lastSlice - firstSlice, 0);

	const medialSurface & rf = *mpn.srf;

	for (size_t ip = mpn.firstPores; ip < mpn.poreIs.size(); ++ip) {
		const poreNE* pr = mpn.poreIs[ip];
		if (pr && pr->mb) {
			int x = pr->mb->fi, y = pr->mb->fj, z = pr->mb->fk;
			float rlim = pr->mb->R * pr->mb->R;
			int ex = 1 * sqrt(rlim);
			int elId = ip;

			for (int a = -ex; a <= ex; ++a)  {
				int ey = sqrt(rlim - a * a);
				for (int b = -ey; b <= ey; ++b)  {
					int ez = sqrt(rlim - a * a - b * b);
					for (int c = -ez; c <= ez; ++c) {
						if (rf.isInside(x + a, y + b, z + c) && (z + c >= firstSlice && z + c < lastSlice)
								&& vfild(x + a, y + b, (z - firstSlice) + c) == 0) {
							vfild(x + a, y + b, z - firstSlice + c) = elId;
						}
					}
				}
			}
		}
	}
	return vfild;
}


voxelField<int> throatMaxBalls(const blockNetwork& mpn)  {
	cout<< " write_throatMaxBalls   "<<endl;
	voxelField<int> vfild(mpn.cg.nx,mpn.cg.ny,mpn.cg.nz,0);

	const medialSurface & rf = *mpn.srf;

	for(const auto tr: mpn.throatIs)  {
		const medialBall* vi1 = tr->mb11(),  *vi2 = tr->mb22(), *vi;

		if(vi1 && vi2)  {	// select largest inscribed sphere
			if(vi1->R > vi2->R)  vi = tr->mb11();
			else                 vi = tr->mb22();  }
		else if (vi1)          vi = tr->mb11();
		else                   vi = tr->mb22();

		int x= vi->fi,   y= vi->fj,   z= vi->fk;
		float rlim = vi->R*vi->R;

		/// absorb 2R range balls
		int ex, ey, ez;
		ex = 1*sqrt(rlim);
		for (int a=-ex; a<=ex; ++a)  {
		 ey = sqrt(1*rlim-a*a);
		 for (int b=-ey; b<=ey; ++b)  {
			ez = sqrt(1*rlim-a*a-b*b);
			for (int c=-ez; c<=ez; ++c)
				if (rf.isInside(x+a, y+b, z+c) && vfild(x+a,y+b,z+c) == 0)
					vfild(x+a,y+b,z+c) = tr->tid + 1;
		 }
		}
	}
	return vfild;
}


 voxelField<int> throatMaxBalls(const blockNetwork& mpn, int firstSlice, int lastSlice)  {
	cout<< " write_throatMaxBalls   "<<endl;
	voxelField<int> vfild(mpn.cg.nx,mpn.cg.ny,lastSlice - firstSlice,0);
	const medialSurface & rf = *mpn.srf;

	for(const auto tr: mpn.throatIs)  {
		const medialBall* vi1 = tr->mb11(),  *vi2 = tr->mb22(),  *vi;

		if(vi1 && vi2)  {		//select largest inscribed sphere
			 if(vi1->R > vi2->R)   vi = tr->mb11();
			 else                  vi = tr->mb22();  }
		else if (vi1)     vi = tr->mb11();
		else              vi = tr->mb22();

		int x= vi->fi,   y= vi->fj,   z= vi->fk;
		float rlim = vi->R*vi->R;

		/// absorb 2R range balls
		int ex, ey, ez;
		ex = 1*sqrt(rlim);
		for (int a=-ex; a<=ex; ++a)  {
			ey = sqrt(1*rlim-a*a);
			for (int b=-ey; b<=ey; ++b)  {
				ez = sqrt(1*rlim-a*a-b*b);
				for (int c=-ez; c<=ez; ++c)
					if (rf.isInside(x+a, y+b, z+c) && (z+c >= firstSlice && 
							z+c < lastSlice) && vfild(x+a,y+b,z-firstSlice+c) == 0)
						vfild(x+a,y+b,z-firstSlice+c) = tr->tid + 1;
			 }
		}
	}
	return vfild;
}


voxelField<int> throatCylinders(const blockNetwork& mpn)  {
  /// Paints each throat's cylinder connecting the two adjacent pores, with radius same as throat radius.
  /// Throat centre indices are guaranteed to have the correct index.

  cout<< " WriteCylinders   "<<endl;
  voxelField<int> vfild(mpn.cg.nx, mpn.cg.ny, mpn.cg.nz, 0);

  for(const auto tr: mpn.throatIs) {

    if (tr->e1 < mpn.nBP6 || tr->e2 < mpn.nBP6)
      continue;  // skip boundary throats

    double R = std::max(double(tr->mb22()->R), 0.8);

    if (tr->e1 < 0 || tr->e1 >= mpn.poreIs.size() || tr->e2 < 0 || tr->e2 >= mpn.poreIs.size())
      continue;

    if (!mpn.poreIs[tr->e1]->mb || !mpn.poreIs[tr->e2]->mb)
      continue;

    dbl3 p1 = mpn.poreIs[tr->e1]->node();
    dbl3 p2 = mpn.poreIs[tr->e2]->node();

    double xmin = std::min(p1.x, p2.x) - R;
    double xmax = std::max(p1.x, p2.x) + R;
    double ymin = std::min(p1.y, p2.y) - R;
    double ymax = std::max(p1.y, p2.y) + R;
    double zmin = std::min(p1.z, p2.z) - R;
    double zmax = std::max(p1.z, p2.z) + R;

    int ix_min = std::max(0, int(xmin));
    int ix_max = std::min(mpn.cg.nx - 1, int(xmax + 1.0));
    int iy_min = std::max(0, int(ymin));
    int iy_max = std::min(mpn.cg.ny - 1, int(ymax + 1.0));
    int iz_min = std::max(0, int(zmin));
    int iz_max = std::min(mpn.cg.nz - 1, int(zmax + 1.0));

    dbl3 ab = p2 - p1;
    double ab_sq = ab & ab;
    const medialSurface & rf = *mpn.srf;
    int tid = tr->tid;

    for (int x = ix_min; x <= ix_max; ++x)  {
      for (int y = iy_min; y <= iy_max; ++y)  {
        for (int z = iz_min; z <= iz_max; ++z)  {
          if (rf.isInside(x, y, z))  {
            dbl3 v(x + 0.5, y + 0.5, z + 0.5);
            dbl3 av = v - p1;
            double t = (ab_sq > 1e-12) ? ((av & ab) / ab_sq) : 0.0;
            if (t < 0.0) t = 0.0;
            if (t > 1.0) t = 1.0;
            dbl3 closest = p1 + ab * t; // on cylinder axis
            double distSq = magSqr(v - closest);
            if (distSq <= R*R + 0.5*R)  {
              if (vfild(x, y, z) == 0)  {
                vfild(x, y, z) = tid + 1;
              }
            }
          }
        }
      }
    }
  }
  return vfild;
}
///. } Tom


///@endcond

