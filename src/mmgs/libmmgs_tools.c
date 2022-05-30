/* =============================================================================
**  This file is part of the mmg software package for the tetrahedral
**  mesh modification.
**  Copyright (c) Bx INP/CNRS/Inria/UBordeaux/UPMC, 2004-
**
**  mmg is free software: you can redistribute it and/or modify it
**  under the terms of the GNU Lesser General Public License as published
**  by the Free Software Foundation, either version 3 of the License, or
**  (at your option) any later version.
**
**  mmg is distributed in the hope that it will be useful, but WITHOUT
**  ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
**  FITNESS FOR A PARTICULAR PURPOSE. See the GNU Lesser General Public
**  License for more details.
**
**  You should have received a copy of the GNU Lesser General Public
**  License and of the GNU General Public License along with mmg (in
**  files COPYING.LESSER and COPYING). If not, see
**  <http://www.gnu.org/licenses/>. Please read their terms carefully and
**  use this copy of the mmg distribution only if you accept them.
** =============================================================================
*/

/**
 * \file mmgs/libmmgs_tools.c
 * \brief Tools functions for the mmgs library
 * \author Charles Dapogny (UPMC)
 * \author Cécile Dobrzynski (Bx INP/Inria/UBordeaux)
 * \author Pascal Frey (UPMC)
 * \author Algiane Froehly (Inria/UBordeaux)
 * \version 5
 * \copyright GNU Lesser General Public License.
 * \todo Doxygen documentation
 */

#include "mmgs.h"
#include "inlined_functions.h"

void MMGS_setfunc(MMG5_pMesh mesh,MMG5_pSol met) {
  if ( (!mesh->info.ani) && ((!met) || (met->size < 6)) ) {
    MMG5_calelt      = MMG5_caltri_iso;
    MMGS_doSol       = MMGS_doSol_iso;
    MMG5_lenSurfEdg  = MMG5_lenSurfEdg_iso;
    MMG5_compute_meanMetricAtMarkedPoints = MMG5_compute_meanMetricAtMarkedPoints_iso;
    MMGS_defsiz      = MMGS_defsiz_iso;
    MMGS_gradsiz     = MMG5_gradsiz_iso;
    MMGS_gradsizreq  = MMG5_gradsizreq_iso;
    intmet           = intmet_iso;
    movintpt         = movintpt_iso;
    movridpt         = movridpt_iso;
  }
  else {
    /* Force data consistency: if aniso metric is provided, met->size==6 and
     * info.ani==0; with -A option, met->size==1 and info.ani==1 */
    met->size = 6;
    mesh->info.ani = 1;

    /* Set function pointers */
    if ( (!met->m) && (!mesh->info.optim) && mesh->info.hsiz<=0. ) {
      MMG5_calelt     = MMG5_caltri_iso;
      MMGS_doSol      = MMGS_doSol_iso;
      MMG5_lenSurfEdg = MMG5_lenSurfEdg_iso;
    }
    else {
      MMG5_calelt     = MMG5_caltri_ani;
      MMGS_doSol      = MMGS_doSol_ani;
      MMG5_lenSurfEdg = MMG5_lenSurfEdg_ani;
    }
    MMG5_compute_meanMetricAtMarkedPoints = MMG5_compute_meanMetricAtMarkedPoints_ani;
    MMGS_defsiz      = MMGS_defsiz_ani;
    MMGS_gradsiz     = MMGS_gradsiz_ani;
    MMGS_gradsizreq  = MMG5_gradsizreq_ani;
    intmet        = intmet_ani;
    movintpt      = movintpt_ani;
    movridpt      = movridpt_ani;
  }
}

int MMGS_usage(char *prog) {

  /* Common generic options, file options and mode options */
  MMG5_mmgUsage(prog);

  /* Common parameters (first section) */
  MMG5_paramUsage1( );

  /* Specific options */
  fprintf(stdout,"-keep-ref    preserve initial domain references in level-set mode.\n");

#ifdef USE_SCOTCH
  fprintf(stdout,"-rn [n]      Turn on or off the renumbering using SCOTCH [0/1] \n");
#endif

  fprintf(stdout,"\n");

  /* Common parameters (second section) */
  MMG5_paramUsage2();

  /* Common options for advanced users */
  MMG5_advancedUsage();

  fprintf(stdout,"\n\n");

  return 1;
}

int MMGS_defaultValues(MMG5_pMesh mesh) {

  MMG5_mmgDefaultValues(mesh);
#ifdef USE_SCOTCH
  fprintf(stdout,"SCOTCH renumbering                  : enabled\n");
#else
  fprintf(stdout,"SCOTCH renumbering                  : disabled\n");
#endif
  fprintf(stdout,"\n\n");

  return 1;
}

// In ls mode : metric must be provided using -met option (-sol or default is the ls).
// In adp mode : -sol or -met or default allow to store the metric.
int MMGS_parsar(int argc,char *argv[],MMG5_pMesh mesh,MMG5_pSol met,MMG5_pSol sol) {
  MMG5_pSol tmp = NULL;
  int    i;
  char   namein[MMG5_FILESTR_LGTH];

  /* First step: search if user want to see the default parameters values. */
  for ( i=1; i< argc; ++i ) {
    if ( !strcmp(argv[i],"-val") ) {
      MMGS_defaultValues(mesh);
      return 0;
    }
  }

  /* Second step: read all other arguments. */
  i = 1;
  while ( i < argc ) {
    if ( *argv[i] == '-' ) {
      switch(argv[i][1]) {
      case '?':
        MMGS_usage(argv[0]);
        return 0;
        break;
      case 'a': /* ridge angle */
        if ( !strcmp(argv[i],"-ar") && ++i < argc ) {
          if ( !MMGS_Set_dparameter(mesh,met,MMGS_DPARAM_angleDetection,
                                    atof(argv[i])) )
            return 0;
        }
        break;
      case 'A': /* anisotropy */
        if ( !MMGS_Set_solSize(mesh,met,MMG5_Vertex,0,MMG5_Tensor) )
          return 0;
        break;
      case 'h':
        if ( !strcmp(argv[i],"-hmin") && ++i < argc ) {
          if ( !MMGS_Set_dparameter(mesh,met,MMGS_DPARAM_hmin,
                                    atof(argv[i])) )
            return 0;
        }
        else if ( !strcmp(argv[i],"-hmax") && ++i < argc ) {
          if ( !MMGS_Set_dparameter(mesh,met,MMGS_DPARAM_hmax,
                                    atof(argv[i])) )
            return 0;
        }
        else if ( !strcmp(argv[i],"-hsiz") && ++i < argc ) {
          if ( !MMGS_Set_dparameter(mesh,met,MMGS_DPARAM_hsiz,
                                    atof(argv[i])) )
            return 0;

        }
        else if ( !strcmp(argv[i],"-hausd") && ++i <= argc ) {
          if ( !MMGS_Set_dparameter(mesh,met,MMGS_DPARAM_hausd,
                                    atof(argv[i])) )
            return 0;
        }
        else if ( !strcmp(argv[i],"-hgradreq") && ++i <= argc ) {
          if ( !MMGS_Set_dparameter(mesh,met,MMGS_DPARAM_hgradreq,
                                    atof(argv[i])) )
            return 0;
        }
        else if ( !strcmp(argv[i],"-hgrad") && ++i <= argc ) {
          if ( !MMGS_Set_dparameter(mesh,met,MMGS_DPARAM_hgrad,
                                    atof(argv[i])) )
            return 0;
        }
        else {
          MMGS_usage(argv[0]);
          return 0;
        }
        break;
      case 'd':
        if ( !strcmp(argv[i],"-default") ) {
          mesh->mark=1;
        }
        else {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_debug,1) )
            return 0;
        }
        break;
      case 'i':
        if ( !strcmp(argv[i],"-in") ) {
          if ( ++i < argc && isascii(argv[i][0]) && argv[i][0]!='-') {
            if ( !MMGS_Set_inputMeshName(mesh, argv[i]) )
              return 0;

            if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_verbose,5) )
              return 0;
          }else{
            fprintf(stderr,"Missing filname for %c%c\n",argv[i-1][1],argv[i-1][2]);
            MMGS_usage(argv[0]);
            return 0;
          }
        }
        break;
      case 'k':
        if ( !strcmp(argv[i],"-keep-ref") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_keepRef,1) )
            return 0;
        }
        break;
      case 'l':
        if ( !strcmp(argv[i],"-ls") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_iso,1) )
            return 0;
          if ( ++i < argc && (isdigit(argv[i][0]) ||
                              (argv[i][0]=='-' && isdigit(argv[i][1])) ) ) {
            if ( !MMGS_Set_dparameter(mesh,met,MMGS_DPARAM_ls,atof(argv[i])) )
              return 0;
          }
          else i--;
        }
        break;
      case 'm':
        if ( !strcmp(argv[i],"-met") ) {
          if ( !met ) {
            fprintf(stderr,"No metric structure allocated for %c%c%c option\n",
                    argv[i-1][1],argv[i-1][2],argv[i-1][3]);
            return 0;
          }
          if ( ++i < argc && isascii(argv[i][0]) && argv[i][0]!='-' ) {
            if ( !MMGS_Set_inputSolName(mesh,met,argv[i]) )
              return 0;
          }
          else {
            fprintf(stderr,"Missing filname for %c%c%c\n",argv[i-1][1],argv[i-1][2],argv[i-1][3]);
            MMGS_usage(argv[0]);
            return 0;
          }
        }
        else if  (!strcmp(argv[i],"-m") ) {
        if ( ++i < argc && isdigit(argv[i][0]) ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_mem,atoi(argv[i])) )
            return 0;
        }
        else {
          fprintf(stderr,"Missing argument option %c\n",argv[i-1][1]);
          MMGS_usage(argv[0]);
          return 0;
        }
        }
        break;
      case 'n':
        if ( !strcmp(argv[i],"-nr") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_angle,0) )
            return 0;
        }
        else if ( !strcmp(argv[i],"-nsd") ) {
          if ( ++i < argc && isdigit(argv[i][0]) ) {
            if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_numsubdomain,atoi(argv[i])) )
              return 0;
          }
          else {
            fprintf(stderr,"Missing argument option %c\n",argv[i-1][1]);
            MMGS_usage(argv[0]);
            return 0;
          }
        }
        else if ( !strcmp(argv[i],"-noswap") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_noswap,1) )
            return 0;
        }
        else if( !strcmp(argv[i],"-noinsert") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_noinsert,1) )
            return 0;
        }
        else if( !strcmp(argv[i],"-nomove") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_nomove,1) )
            return 0;
        }
        else if ( !strcmp(argv[i],"-nreg") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_nreg,1) )
            return 0;
        }
        else if( !strcmp(argv[i],"-nosizreq") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_nosizreq,1) ) {
            return 0;
          }
        }
        break;
      case 'o':
        if ( (!strcmp(argv[i],"-out")) || (!strcmp(argv[i],"-o")) ) {
          if ( ++i < argc && isascii(argv[i][0])  && argv[i][0]!='-') {
            if ( !MMGS_Set_outputMeshName(mesh,argv[i]) )
              return 0;
          }else{
            fprintf(stderr,"Missing filname for %c%c%c\n",
                    argv[i-1][1],argv[i-1][2],argv[i-1][3]);
            MMGS_usage(argv[0]);
            return 0;
          }
        }
        else if( !strcmp(argv[i],"-optim") ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_optim,1) )
            return 0;
        }
        else {
          fprintf(stderr,"Unrecognized option %s\n",argv[i]);
          MMGS_usage(argv[0]);
          return 0;
        }
        break;
#ifdef USE_SCOTCH
      case 'r':
        if ( !strcmp(argv[i],"-rn") ) {
          if ( ++i < argc ) {
            if ( isdigit(argv[i][0]) ) {
              if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_renum,atoi(argv[i])) )
                return 0;
            }
            else {
              fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
              MMGS_usage(argv[0]);
              return 0;
            }
          }
          else {
            fprintf(stderr,"Missing argument option %s\n",argv[i-1]);
            MMGS_usage(argv[0]);
            return 0;
          }
        }
        break;
#endif
      case 's':
        if ( !strcmp(argv[i],"-sol") ) {
          /* For retrocompatibility, store the metric if no sol structure available */
          tmp = sol ? sol : met;
          assert(tmp);
          if ( ++i < argc && isascii(argv[i][0]) && argv[i][0]!='-' ) {
            if ( !MMGS_Set_inputSolName(mesh,tmp,argv[i]) )
              return 0;
          }
          else {
            fprintf(stderr,"Missing filname for %c%c%c\n",argv[i-1][1],argv[i-1][2],argv[i-1][3]);
            MMGS_usage(argv[0]);
            return 0;
          }
        }
        break;
      case 'v':
        if ( ++i < argc ) {
          if ( argv[i][0] == '-' || isdigit(argv[i][0]) ) {
            if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_verbose,atoi(argv[i])) )
              return 0;
          }
          else
            i--;
        }
        else {
          fprintf(stderr,"Missing argument option %c\n",argv[i-1][1]);
          MMGS_usage(argv[0]);
          return 0;
        }
        break;
      default:
        fprintf(stderr,"Unrecognized option %s\n",argv[i]);
        MMGS_usage(argv[0]);
        return 0;
      }
    }
    else {
      if ( mesh->namein == NULL ) {
        if ( !MMGS_Set_inputMeshName(mesh,argv[i]) )
          return 0;
        if ( mesh->info.imprim == -99 ) {
          if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_verbose,5) )
            return 0;
        }
      }
      else if ( mesh->nameout == NULL ) {
        if ( !MMGS_Set_outputMeshName(mesh,argv[i]) )
          return 0;
      }
      else {
        fprintf(stdout,"Argument %s ignored\n",argv[i]);
        MMGS_usage(argv[0]);
        return 0;
      }
    }
    i++;
  }

  /* check file names */
  if ( mesh->info.imprim == -99 ) {
    fprintf(stdout,"\n  -- PRINT (0 10(advised) -10) ?\n");
    fflush(stdin);
    MMG_FSCANF(stdin,"%d",&i);
    if ( !MMGS_Set_iparameter(mesh,met,MMGS_IPARAM_verbose,i) )
      return 0;
  }

  if ( mesh->namein == NULL ) {
    fprintf(stdout,"  -- INPUT MESH NAME ?\n");
    fflush(stdin);
    MMG_FSCANF(stdin,"%127s",namein);
    if ( !MMGS_Set_inputMeshName(mesh,namein) )
      return 0;
  }

  if ( mesh->nameout == NULL ) {
    if ( !MMGS_Set_outputMeshName(mesh,"") )
      return 0;
  }

  /* adp mode: if the metric name has been stored in sol, move it in met */
  if ( met->namein==NULL && sol && sol->namein && !(mesh->info.iso || mesh->info.lag>=0) ) {
    if ( !MMGS_Set_inputSolName(mesh,met,sol->namein) )
      return 0;
    MMG5_DEL_MEM(mesh,sol->namein);
  }

  /* default : store solution name in iso mode, metric name otherwise */
  tmp = ( mesh->info.iso || mesh->info.lag >=0 ) ? sol : met;
  assert ( tmp );
  if ( tmp->namein == NULL ) {
    if ( !MMGS_Set_inputSolName(mesh,tmp,"") ) { return 0; }
  }
  if ( met->nameout == NULL ) {
    if ( !MMGS_Set_outputSolName(mesh,met,"") )
      return 0;
  }
  return 1;
}

int MMGS_freeLocalPar(MMG5_pMesh mesh) {

  free(mesh->info.par);
  mesh->info.npar = 0;

  return 1;
}

int MMGS_stockOptions(MMG5_pMesh mesh, MMG5_Info *info) {

  memcpy(&mesh->info,info,sizeof(MMG5_Info));
  MMGS_memOption(mesh);
  if( mesh->info.mem > 0) {
    if ( mesh->npmax < mesh->np || mesh->ntmax < mesh->nt ) {
      return 0;
    } else if(mesh->info.mem < 39)
      return 0;
  }
  return 1;
}

void MMGS_destockOptions(MMG5_pMesh mesh, MMG5_Info *info) {

  memcpy(info,&mesh->info,sizeof(MMG5_Info));
  return;
}

int MMGS_Get_numberOfNonBdyEdges(MMG5_pMesh mesh, int* nb_edges) {
  MMG5_pTria pt,pt1;
  MMG5_pEdge ped;
  int        *adja,k,i,j,i1,i2,iel;

  *nb_edges = 0;
  if ( mesh->tria ) {
    /* Create the triangle adjacency if needed */
    if ( !mesh->adja ) {
      if ( !MMGS_hashTria(mesh) ) {
        fprintf(stderr,"\n  ## Error: %s: unable to create "
                "adjacency table.\n",__func__);
        return 0;
      }
    }

    /* Count the number of non boundary edges */
    for ( k=1; k<=mesh->nt; k++ ) {
      pt = &mesh->tria[k];
      if ( !MG_EOK(pt) ) continue;

      adja = &mesh->adja[3*(k-1)+1];

      for ( i=0; i<3; i++ ) {
        /* Do not treat boundary edges */
        if ( MG_EDG(pt->tag[i]) ) continue;

        iel = adja[i] / 3;
        assert ( iel != k );

        pt1 = &mesh->tria[iel];

        if ( (!iel) || (pt->ref != pt1->ref) ) {
          /* Do not treat boundary edges */
          continue;
        }
        if ( k < iel ) {
          /* Treat edge from the triangle with lowest index */
          ++(*nb_edges);
        }
      }
    }

    /* Append the non boundary edges to the boundary edges array */
    if ( mesh->na ) {
      MMG5_ADD_MEM(mesh,(*nb_edges)*sizeof(MMG5_Edge),"non boundary edges",
                   printf("  Exit program.\n");
                   return 0);
      MMG5_SAFE_RECALLOC(mesh->edge,(mesh->na+1),(mesh->na+(*nb_edges)+1),
                         MMG5_Edge,"non bdy edges arrray",return 0);
    }
    else {
      MMG5_ADD_MEM(mesh,((*nb_edges)+1)*sizeof(MMG5_Edge),"non boundary edges",
                   printf("  Exit program.\n");
                   return 0);
      MMG5_SAFE_RECALLOC(mesh->edge,0,(*nb_edges)+1,
                         MMG5_Edge,"non bdy edges arrray",return 0);
    }

    j = mesh->na+1;
    for ( k=1; k<=mesh->nt; k++ ) {
      pt = &mesh->tria[k];
      if ( !MG_EOK(pt) ) continue;

      adja = &mesh->adja[3*(k-1)+1];

      for ( i=0; i<3; i++ ) {
        /* Do not treat boundary edges */
        if ( MG_EDG(pt->tag[i]) ) continue;

        i1 = MMG5_inxt2[i];
        i2 = MMG5_iprv2[i];
        iel = adja[i] / 3;
        assert ( iel != k );

        pt1 = &mesh->tria[iel];

        if ( (!iel) || (pt->ref != pt1->ref) ) {
          /* Do not treat boundary edges */
          continue;
        }
        if ( k < iel ) {
          /* Treat edge from the triangle with lowest index */
          ped = &mesh->edge[j++];
          assert ( ped );
          ped->a   = pt->v[i1];
          ped->b   = pt->v[i2];
          ped->ref = pt->edg[i];
        }
      }
    }
  }
  return 1;
}

int MMGS_Get_nonBdyEdge(MMG5_pMesh mesh, int* e0, int* e1, int* ref, int idx) {
  MMG5_pEdge ped;
  size_t     na_tot=0;
  char       *ptr_c = (char*)mesh->edge;

  if ( !mesh->edge ) {
    fprintf(stderr,"\n  ## Error: %s: edge array is not allocated.\n"
            " Please, call the MMGS_Get_numberOfNonBdyEdges function"
            " before the %s one.\n",
            __func__,__func__);
    return 0;
  }

  ptr_c = ptr_c-sizeof(size_t);
  na_tot = *((size_t*)ptr_c);

  if ( mesh->namax==na_tot ) {
    fprintf(stderr,"\n  ## Error: %s: no internal edge.\n"
            " Please, call the MMGS_Get_numberOfNonBdyEdges function"
            " before the %s one and check that the number of internal"
            " edges is non null.\n",
            __func__,__func__);
  }

  if ( mesh->namax+idx > na_tot ) {
    fprintf(stderr,"\n  ## Error: %s: Can't get the internal edge of index %d."
            " Index must be between 1 and %zu.\n",
            __func__,idx,na_tot-mesh->namax);
  }

  ped = &mesh->edge[mesh->na+idx];

  *e0  = ped->a;
  *e1  = ped->b;

  if ( ref != NULL ) {
    *ref = mesh->edge[mesh->na+idx].ref;
  }

  return 1;
}

int MMGS_Get_adjaTri(MMG5_pMesh mesh, int kel, int listri[3]) {

  if ( ! mesh->adja ) {
    if (! MMGS_hashTria(mesh))
      return 0;
  }

  listri[0] = mesh->adja[3*(kel-1)+1]/3;
  listri[1] = mesh->adja[3*(kel-1)+2]/3;
  listri[2] = mesh->adja[3*(kel-1)+3]/3;

  return 1;
}

int MMGS_Get_adjaVerticesFast(MMG5_pMesh mesh, int ip,int start, int lispoi[MMGS_LMAX])
{
  MMG5_pTria pt;
  int k,prevk,nbpoi,iploc,i,i1,i2,*adja;

  pt   = &mesh->tria[start];

  for ( iploc=0; iploc<3; ++iploc ) {
    if ( pt->v[iploc] == ip ) break;
  }

  assert(iploc!=3);

  k = start;
  i = iploc;
  nbpoi = 0;
  do {
    if ( nbpoi == MMGS_LMAX ) {
      fprintf(stderr,"\n  ## Warning: %s: unable to compute adjacent"
              " vertices of the vertex %d:\nthe ball of point contain too many"
              " elements.\n",__func__,ip);
      return 0;
    }
    i1 = MMG5_inxt2[i];
    lispoi[nbpoi] = mesh->tria[k].v[i1];
    ++nbpoi;

    adja = &mesh->adja[3*(k-1)+1];
    prevk = k;
    k  = adja[i1] / 3;
    i  = adja[i1] % 3;
    i  = MMG5_inxt2[i];
  }
  while ( k && k != start );

  if ( k > 0 ) return nbpoi;

  /* store the last point of the boundary triangle */
  if ( nbpoi == MMGS_LMAX ) {
    fprintf(stderr,"\n  ## Warning: %s: unable to compute adjacent vertices of the"
            " vertex %d:\nthe ball of point contain too many elements.\n",
            __func__,ip);
    return 0;
  }
  i1 = MMG5_inxt2[i1];
  lispoi[nbpoi] = mesh->tria[prevk].v[i1];
  ++nbpoi;

  /* check if boundary hit */
  k = start;
  i = iploc;
  do {
    adja = &mesh->adja[3*(k-1)+1];
    i2 = MMG5_iprv2[i];
    k  = adja[i2] / 3;
    if ( k == 0 )  break;

    if ( nbpoi == MMGS_LMAX ) {
      fprintf(stderr,"\n  ## Warning: %s: unable to compute adjacent vertices of the"
              " vertex %d:\nthe ball of point contain too many elements.\n",
              __func__,ip);
      return 0;
    }
    i  = adja[i2] % 3;
    lispoi[nbpoi] = mesh->tria[k].v[i];
    ++nbpoi;

    i  = MMG5_iprv2[i];
  }
  while ( k );

  return nbpoi;
}

/**
 * \param mesh pointer toward the mesh structure.
 * \param met pointer toward the solution structure.
 * \param ani 1 for aniso metric, 0 for iso one
 *
 * \return 0 if fail, 1 if succeed.
 *
 * Truncate the metric computed by the DoSol function by hmax and hmin values
 * (if setted by the user). Set hmin and hmax if they are not setted.
 *
 */
static inline
int MMGS_solTruncatureForOptim(MMG5_pMesh mesh, MMG5_pSol met,int ani) {
  int         i,k,ier;

  assert ( mesh->info.optim );

  /* Detect the point not used by the mesh */
  for (k=1; k<=mesh->np; k++) {
    mesh->point[k].flag = 1;
  }

  for (k=1; k<=mesh->nt; k++) {
    MMG5_pTria ptt = &mesh->tria[k];
    if ( !MG_EOK(ptt) ) continue;

    for (i=0; i<3; i++) {
      mesh->point[ptt->v[i]].flag = 0;
    }
  }

  /* Compute hmin/hmax on unflagged points and truncate the metric */
  if ( !ani ) {
    ier = MMG5_solTruncature_iso(mesh,met);
  }
  else {
    ier = MMG5_3dSolTruncature_ani(mesh,met);
  }

  return ier;
}


/**
 * \param mesh pointer toward the mesh
 * \param met pointer toward the metric
 *
 * \return 1 if succeed, 0 if fail
 *
 * \remark need the normal at vertices.
 *
 * Compute isotropic size map according to the mean of the length of the
 * edges passing through a point.
 *
 */
int MMGS_doSol_iso(MMG5_pMesh mesh,MMG5_pSol met) {
  MMG5_pTria   ptt;
  MMG5_pPoint  p1,p2;
  double       ux,uy,uz,dd;
  int          i,k,ipa,ipb,type;
  int          *mark;

  MMG5_SAFE_CALLOC(mark,mesh->np+1,int,return 0);

  /* Memory alloc */
  if ( met->size!=1 ) {
    fprintf(stderr,"\n  ## Error: %s: unexpected size of metric: %d.\n",
            __func__,met->size);
    return 0;
  }

  type = 1;
  if ( !MMGS_Set_solSize(mesh,met,MMG5_Vertex,mesh->np,type) )
    return 0;

  /* Travel the triangles edges and add the edge contribution to edges
   * extermities */
  for (k=1; k<=mesh->nt; k++) {
    ptt = &mesh->tria[k];
    if ( !MG_EOK(ptt) )  continue;

    for (i=0; i<3; i++) {
      ipa = ptt->v[i];
      ipb = ptt->v[MMG5_inxt2[i]];
      p1  = &mesh->point[ipa];
      p2  = &mesh->point[ipb];

      ux  = p1->c[0] - p2->c[0];
      uy  = p1->c[1] - p2->c[1];
      uz  = p1->c[2] - p2->c[2];
      dd  = sqrt(ux*ux + uy*uy + uz*uz);

      met->m[ipa] += dd;
      mark[ipa]++;
      met->m[ipb] += dd;
      mark[ipb]++;
    }
  }

  /* vertex size */
  for (k=1; k<=mesh->np; k++) {
    if ( !mark[k] ) {
      continue;
    }
    else
      met->m[k] = met->m[k] / (double)mark[k];
  }

  MMG5_SAFE_FREE(mark);

  MMGS_solTruncatureForOptim(mesh,met,0);

  if ( mesh->info.iso ) {
    /* Clean spurious entities added by mesh analysis */
    if ( !MMGS_clean_analys_for_norver(mesh) ) {
      return 0;
    }
  }


  return 1;
}

int MMGS_doSol_ani(MMG5_pMesh mesh,MMG5_pSol met) {
  MMG5_pTria   ptt;
  MMG5_pPoint  p1;
  double       dd,tensordot[3];
  double       *m,r[3][3],lispoi[3*MMGS_LMAX+1],b0[3],b1[3],b2[3];
  int          i,j,k,iadr,ip,type,list[MMGS_LMAX+2],ilist;
  int          *mark;

  if ( mesh->info.iso ) {
    /* Mesh analysis: normal at vertices are needed. In adaptation mode,
     * analysis has already been computed */
    if ( !MMGS_analys_for_norver(mesh) ) {
      return 0;
    }
  }

  MMG5_SAFE_CALLOC(mark,mesh->np+1,int,return 0);

  /* Memory alloc */
  if ( met->size!=6 ) {
    fprintf(stderr,"\n  ## Error: %s: unexpected size of metric: %d.\n",
            __func__,met->size);
    return 0;
  }

  type = 3;
  if ( !MMGS_Set_solSize(mesh,met,MMG5_Vertex,mesh->np,type) ) {
    fprintf(stderr,"\n  ## Error: %s: unable to allocate metric.\n",
            __func__);
    return 0;
  }

  /* Travel the triangles */
#warning to remove
  double isqhmax = 1./(mesh->info.hmax*mesh->info.hmax);
  for (k=1; k<=mesh->nt; k++) {
    ptt = &mesh->tria[k];
    if ( !MG_EOK(ptt) )  continue;

    for (i=0; i<3; i++) {
      ip = ptt->v[i];

      if ( mark[ip] ) {
        continue;
      }
      p1  = &mesh->point[ip];

      /* Regular point: get the ball of the point and project it in the tangent
       * plane to compute the unit metric tensor in 2D. */
      ilist = boulet(mesh,k,i,list);
      if ( ilist < 1 ) {
        fprintf(stderr,"\n  ## Error: %s: unable to compute ball of point.\n",
                __func__);
        return 0;
      }

      mark[ip] = ilist;

      iadr = 6*ip;
      m    = &met->m[iadr];

      if ( p1->tag & MG_CRN || p1->tag & MG_GEO || p1->tag & MG_REF ) {
#warning to Treat
        m[0] = m[3] = m[5] = isqhmax;
        continue;
      }

      /* Rotation of the ball of p0 so lispoi will contain all the points of the
         ball of p0, rotated so that t_{p_0}S = [z = 0] */
      if ( !MMGS_surfballRotation(mesh,p1,list,ilist,r,lispoi)  ) {
        fprintf(stderr,"\n  ## Error: %s: unable to compute ball rotation.\n",
                __func__);
        return 0;
      }

      tensordot[0] = 0.;
      tensordot[1] = 0.;
      tensordot[2] = 0.;

      for ( j=0; j<ilist; ++j ) {
        /* Compute the 2D metric tensor from the projection of the vectors on
         * the tangent plane */
        tensordot[0] += lispoi[3*j+1]*lispoi[3*j+1];
        tensordot[1] += lispoi[3*j+1]*lispoi[3*j+2];
        tensordot[2] += lispoi[3*j+2]*lispoi[3*j+2];
      }

      /* Metric = nedges/dim * inv (sum(tensor_dot(edges,edges)))  */
      dd = 1./(tensordot[0]*tensordot[2] - tensordot[1]*tensordot[1]);
      dd *= (double)ilist/2.;

      double tmp = tensordot[0];

      tensordot[0] = dd*tensordot[2];
      tensordot[1] = -dd*tensordot[1];
      tensordot[2] = dd*tmp;

#ifndef NDEBUG
      /* Check 2D metric */
      double lambda[2],vp[2][2];
      MMG5_eigensym(tensordot,lambda,vp);

      assert (lambda[0] > 0. && lambda[1] > 0. && "Negative eigenvalue");

      /* Normally the case where the point belongs to only 2 colinear points is
         impossible */
      assert (isfinite(lambda[0]) && isfinite(lambda[1]) && "wrong eigenvalue");
#endif

#warning to factorize
      /* At this point, tensordot (with 0 replaced by isqhmax in the z
         direction) is the desired metric, except it is expressed in the rotated
         canonical basis, that is tensordot = R * metric in cb * ^t R, so metric
         in cb = ^tR*intm*R */
      // intm = intm[0]  intm[1]    0
      //        intm[1]  intm[2]    0
      //           0       0     isqhmax

      /* b0 and b1 serve now for nothing : let them be the lines of matrix intm*R */
      b0[0] = tensordot[0]*r[0][0] + tensordot[1]*r[1][0];
      b0[1] = tensordot[0]*r[0][1] + tensordot[1]*r[1][1];
      b0[2] = tensordot[0]*r[0][2] + tensordot[1]*r[1][2];
      b1[0] = tensordot[1]*r[0][0] + tensordot[2]*r[1][0];
      b1[1] = tensordot[1]*r[0][1] + tensordot[2]*r[1][1];
      b1[2] = tensordot[1]*r[0][2] + tensordot[2]*r[1][2];
      b2[0] = isqhmax*r[2][0];
      b2[1] = isqhmax*r[2][1];
      b2[2] = isqhmax*r[2][2];

      m[0] = r[0][0] * b0[0] + r[1][0] * b1[0] + r[2][0] * b2[0];
      m[1] = r[0][0] * b0[1] + r[1][0] * b1[1] + r[2][0] * b2[1];
      m[2] = r[0][0] * b0[2] + r[1][0] * b1[2] + r[2][0] * b2[2];

      m[3] = r[0][1] * b0[1] + r[1][1] * b1[1] + r[2][1] * b2[1];
      m[4] = r[0][1] * b0[2] + r[1][1] * b1[2] + r[2][1] * b2[2];

      m[5] = r[0][2] * b0[2] + r[1][2] * b1[2] + r[2][2] * b2[2];
    }
  }

  MMG5_SAFE_FREE(mark);

  MMGS_solTruncatureForOptim(mesh,met,1);

  return 1;
}

int MMGS_Set_constantSize(MMG5_pMesh mesh,MMG5_pSol met) {
  double      hsiz;
  int         type;

  /* Set solution size */
  if ( mesh->info.ani ) {
    met->size = 6;
    type = 3;
  }
  else {
    met->size = 1;
    type = 1;
  }

  /* Memory alloc */
  if ( !MMGS_Set_solSize(mesh,met,MMG5_Vertex,mesh->np,type) )
    return 0;

  if ( !MMG5_Compute_constantSize(mesh,met,&hsiz) )
    return 0;

  mesh->info.hsiz = hsiz;

  MMG5_Set_constantSize(mesh,met,hsiz);

  return 1;
}

int MMGS_Compute_eigenv(double m[6],double lambda[3],double vp[3][3]) {

  return  MMG5_eigenv(1,m,lambda,vp);

}

void MMGS_Free_solutions(MMG5_pMesh mesh,MMG5_pSol sol) {

  /* sol */
  if ( !sol ) return;

  if ( sol->m )
    MMG5_DEL_MEM(mesh,sol->m);

  if ( sol->namein ) {
    MMG5_DEL_MEM(mesh,sol->namein);
  }

  if ( sol->nameout ) {
    MMG5_DEL_MEM(mesh,sol->nameout);
  }

  memset ( sol, 0x0, sizeof(MMG5_Sol) );

  /* Reset state to a scalar status */
  sol->dim  = 3;
  sol->ver  = 2;
  sol->size = 1;
  sol->type = 1;

  return;
}
