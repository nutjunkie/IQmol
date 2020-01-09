! /*******************************************************************************
!        
!   Copyright (C) 2011-2015 Andrew Gilbert
!            
!   This file is part of IQmol, a free molecular visualization program. See
!   <http://iqmol.org> for more details.
!        
!   IQmol is free software: you can redistribute it and/or modify it under the
!   terms of the GNU General Public License as published by the Free Software
!   Foundation, either version 3 of the License, or (at your option) any later
!   version.
!          
!   IQmol is distributed in the hope that it will be useful, but WITHOUT ANY
!   WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
!   FOR A PARTICULAR PURPOSE.  See the GNU General Public License for more
!   details.
!        
!  You should have received a copy of the GNU General Public License along
!  with IQmol.  If not, see <http://www.gnu.org/licenses/>.  
!        
! ********************************************************************************/

!                            SYMMOL
!         A PROGRAM FOR THE SYMMETRIZATION OF GROUPS OF ATOMS
!              By Tullio Pilati and Alessandra Forni
!                   Version November 4th 2002
!
!     This version modified and converted to f90 by Andrew Gilbert 2011



!     -----------------------------------------------------------------
!     PROGRAM Main
      SUBROUTINE MainTest
!     -----------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER (NMA = 100)
      DIMENSION coordinates(3,NMA)
      INTEGER atomicNumbers(NMA)
      CHARACTER*3 pointGroup
 
      READ(*,*) nAtoms
      IF (nAtoms .gt. NMA) STOP 'Too many atoms in Symmol'
      READ(*,*) tolerance
      DO i = 1,nAtoms
         READ (*,*) atomicNumbers(i),  (coordinates(k,i), k=1,3)
         IF (atomicNumbers(i) .gt. 103) STOP 'Z too high in SymMol'
      ENDDO

      CALL SymMol(nAtoms, Tolerance,  Coordinates, atomicNumbers, pointgroup)

      END



!     -----------------------------------------------------------------
      MODULE GlobalArrays
!     -----------------------------------------------------------------

      INTEGER :: natoms
      INTEGER :: nmg, nma, nmv

      PARAMETER (nmg = 120)
      PARAMETER (nma = 1000)
      PARAMETER (nmv = nma*nmg)

      REAL*8, DIMENSION (:,:), ALLOCATABLE :: X
      REAL*8, DIMENSION (:),   ALLOCATABLE :: AMAS
      REAL*8, DIMENSION (:),   ALLOCATABLE :: MSP
      REAL*8, DIMENSION (:,:), ALLOCATABLE :: SX
      REAL*8, DIMENSION (:),   ALLOCATABLE :: SIG
      REAL*8, DIMENSION (:),   ALLOCATABLE :: DXM
      REAL*8, DIMENSION (:),   ALLOCATABLE :: MLG

      REAL*8 :: PC(7), PCR(7), O(3,3), OI(3,3), G(3,3), GI(3,3), CS(12)
      REAL*8 :: OR(3,3), OT(3,3), OTI(3,3), BARC(3), BARO(3), RIN(3)

!     SIMME
      REAL*8, DIMENSION (:,:,:), ALLOCATABLE :: SIM
      REAL*8, DIMENSION (:),     ALLOCATABLE :: DEV
      REAL*8, DIMENSION (:),     ALLOCATABLE :: CSM
      REAL*8, DIMENSION (:,:),   ALLOCATABLE :: MTG

      REAL*8  :: CSMT
      INTEGER :: NMS

!     SIMME1
      REAL*8, DIMENSION (:,:), ALLOCATABLE :: RMS
      REAL*8 :: RMST

!     RMSMIN
      REAL*8, DIMENSION (:,:), ALLOCATABLE :: ppu
      REAL*8, DIMENSION (:,:), ALLOCATABLE :: ppo

      REAL*8  :: RV(3,3)
      INTEGER :: npu

!     AT2
      REAL*8  :: DCM, DCME
      INTEGER :: indwgh, indtol

      END MODULE



!     -----------------------------------------------------------------
      SUBROUTINE SymMol(nat, Tolerance, coordinates, atomicNumbers, pointGroup)
!     -----------------------------------------------------------------

      USE GlobalArrays
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      CHARACTER*3 pointGroup
      DIMENSION coordinates(3,*)
      INTEGER   atomicNumbers(nAtoms)


      natoms = nat

      ALLOCATE (X(3, natoms))
      ALLOCATE (AMAS(natoms))
      ALLOCATE (MSP(natoms))
      ALLOCATE (SX(3,natoms))
      ALLOCATE (SIG(natoms))
      ALLOCATE (DXM(natoms))
      ALLOCATE (MLG(natoms))

      ALLOCATE (SIM(3, 4, nmg))
      ALLOCATE (DEV(nmg))
      ALLOCATE (CSM(nmg))
      ALLOCATE (MTG(nmg, nmg))

      ALLOCATE (RMS(3,nma))
      ALLOCATE (PPU(3,nmv))
      ALLOCATE (PPO(3,nmv))

!     WRITE(*,*)'                     SYMMOL'
!     WRITE(*,*)' A PROGRAM FOR THE SYMMETRIZATION OF GROUPS OF ATOMS'
!     WRITE(*,*)'       By Tullio Pilati and Alessandra Forni'
!     WRITE(*,*)'               Version November 4th 2002'

!     Hard-wired cell parameters
      PC(1) =  1.0d0
      PC(2) =  1.0d0
      PC(3) =  1.0d0
      PC(4) = 90.0d0
      PC(5) = 90.0d0
      PC(6) = 90.0d0

      pointGroup = 'C1 '

!     Only use weights based on atomic masses
      indwgh = 1
!     Use a constant tolerance (2 => distance based)
      indtol = 1
      DCM  = Tolerance
      DCME = DCM

!     Subsequent lines are all atomic coordinates

      DO i = 1,nAtoms
         X(1,i) = coordinates(1,i)
         X(2,i) = coordinates(2,i)
         X(3,i) = coordinates(3,i)
      ENDDO

      NA = NAtoms

      CALL massdata(atomicNumbers)
      CALL cella
      CALl work(Coordinates, pointGroup)


      WRITE(*,10) Tolerance
   10 FORMAT ('Symmetrized Orthogonal Coordinates Tol = ', F6.3)
!     DO i = 1,nAtoms
!        WRITE(*,'(i2, 3(f16.10))') atomicNumbers(i), (Coordinates(k,I),k=1,3)
!     ENDDO

      DEALLOCATE (X, AMAS, MSP, SX, SIG, DXM, MLG)
      DEALLOCATE (SIM, DEV, CSM, MTG, RMS, PPU, PPO)

      END



!     ------------------------------------------------------------------
      SUBROUTINE asymunit(MK,IASU,N)
!     ------------------------------------------------------------------

      USE GlobalArrays
      DIMENSION MK(NMA,NMG), IASU(NMA)

      DO i = 1,N
        IASU(I) = 2
      ENDDO

      DO 2800 i = 1,N
         if (IASU(i) .ne. 2) GOTO 2800
         DO J = 1,NMS
            K = IABS(MK(I,J))
            IF (K.ne.I) THEN
               IF (K.eq.0) THEN
                  IASU(I) = 0
                  GOTO 2800
               ENDIF
               IASU(K)=1
            ENDIF
         ENDDO

 2800 CONTINUE

      RETURN
      END



      SUBROUTINE ax_order(A,i,m,msign,invers)
!
!     m = group order for the matrix SIM(i)
!     msign = 1 asse di rotazione propria
!     msign =-1 asse di rotazione impropria
!     
      PARAMETER (maxorder=8)
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,3,*), B(3,3), C(3,3,2)

      CALL vrload(C,0.d0,18)

      C(1,1,1) =  1.d0
      C(2,2,1) =  1.d0
      C(3,3,1) =  1.d0
      C(1,1,2) = -1.d0
      C(2,2,2) = -1.d0
      C(3,3,2) = -1.d0

      invers = 2
      msign = NINT(det(A,i))
      CALL prodmm(A,C,B,i,1,1)

      DO m = 1,2*maxorder
         IF (ium(C,B,1.d-2,2,1).eq.1) invers = 1
         IF (ium(C,B,1.d-2,1,1).eq.1) GOTO 1000
         CALL prodmm(A,B,B,i,1,1)
      ENDDO

      write(*,*)'INPUT PARAMETER DCM probably too HIGH. Reduce it!'
      stop

1000  IF (m.lt.6.or.msign.eq.1) RETURN
      m1 = (m/4)*4
      if (m1.ne.m) m = m/invers

      RETURN
      END



!     -----------------------------------------------------------------
      SUBROUTINE vrload(A, const, N)
!     -----------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(N)

      DO I = 1,N
         A(I) = const
      ENDDO

      RETURN
      END



      SUBROUTINE cella
      USE GlobalArrays
      IMPLICIT DOUBLE PRECISION (a-h,o-z)

      RAD    = 57.29577951308232D0
      ARAD   = 1.D0/RAD
      PC(7)  = 1.D0
      PCR(7) = 1.D0
      COM    = 1.D0

      DO 1030 I = 1,3
      K=I+3
      CS(I) = COS(PC(K)*ARAD)
      CS(K) = SIN(PC(K)*ARAD)
      IF(PC(K).GT.0.D0) GOTO 1030
      PC(K) = 90.D0
      CS(I) = 0.D0
      CS(K) = 1.D0
 1030 COM = COM-CS(I)*CS(I)

      COM = SQRT(COM+2.D0*CS(1)*CS(2)*CS(3))
      O(1,1) = PC(1)*CS(6)
      O(1,2) = 0.D0
      O(1,3) = PC(3)*(CS(2)-CS(1)*CS(3))/CS(6)
      O(2,1) = PC(1)*CS(3)
      O(2,2) = PC(2)
      O(2,3) = PC(3)*CS(1)
      O(3,1) = 0.D0
      O(3,2) = 0.D0
      O(3,3) = PC(3)*COM/CS(6)

      IF (ABS(O(2,1)).LT.1.D-10) O(2,1) = 0.D0
      IF (ABS(O(2,3)).LT.1.D-10) O(2,3) = 0.D0
      IF (ABS(O(1,3)).LT.1.D-10) O(1,3) = 0.D0

      PC(7) = det(O,1)
      CALL TransposeMatrix(O,G,1,1)
      CALL prodmm(G,O,G,1,1,1)
      CALL Invert3x3(O,OI,1,1)

      PCR(7) = det(OI,1)

      CALL Invert3x3(G,GI,1,1)

      PCR(1)=SQRT(GI(1,1))
      PCR(2)=SQRT(GI(2,2))
      PCR(3)=SQRT(GI(3,3))

      CS(7)=GI(2,3)/(PCR(2)*PCR(3))
      CS(8)=GI(1,3)/(PCR(1)*PCR(3))
      CS(9)=GI(1,2)/(PCR(1)*PCR(2))

      PCR(4)=RAD*DACOS(CS(7))
      PCR(5)=RAD*DACOS(CS(8))
      PCR(6)=RAD*DACOS(CS(9))
      CS(10)=SIN(ARAD*PCR(4))
      CS(11)=SIN(ARAD*PCR(5))
      CS(12)=SIN(ARAD*PCR(6))

      RETURN
      END



!     ------------------------------------------------------------------
      SUBROUTINE LinearCombination(A,B,C,D,E,I,J,K)
!     ------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,*), B(3,*), C(3,*)

      C(1,K) = A(1,I)*D + B(1,J)*E
      C(2,K) = A(2,I)*D + B(2,J)*E
      C(3,K) = A(3,I)*D + B(3,J)*E

      RETURN
      END


      SUBROUTINE compatta(WORD,L,K)
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      CHARACTER*(*) WORD

      K = 0
      DO 1 I = 1,L
        IF (WORD(I:I) .LE. ' ' .OR. WORD(I:I) .GT. '~') GOTO 1
        K = K+1
        WORD(K:K) = WORD(I:I)
    1 CONTINUE

      N = K+1
      DO I = N,L
         WORD(I:I) = ' '
      ENDDO

      RETURN
      END




!     ------------------------------------------------------------------
      SUBROUTINE CompleteGroup(MK,N,*)
!     ------------------------------------------------------------------

      USE GlobalArrays
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION CO(3,3), MK(NMA,NMG)

 2520 NN = NMS
      DO I = 1,NN
         DO J = 1,NN
            CALL prodmm(SIM,SIM,CO,I,J,1)
            DO JJ = 1,NN
               L = ium(CO,SIM,1.d-2,1,JJ)
               IF(L.EQ.1) GOTO 2590
            ENDDO
            GOTO 2610
 2590       MTG(I,J) = JJ
         ENDDO
      ENDDO

      RETURN 

!     We have found a new matrix
 2610 NMS = NMS+1
      IF (NMS.LE.NMG) GOTO 2630

      WRITE(6,2)
    2 FORMAT(' ERROR: TOO MANY MATRICES FOUND')

      WRITE(6,'(3(3F10.5,/),/)') (((SIM(I,J,K),J=1,3),I=1,3),K=1,NMS)

!     ritorno per errore
      RETURN 1

 2630 CALL MatrixCopy(CO,SIM,1,NMS)

      DO k = 1,N
         k1 = MK(k,J)
         MK(k,NMS) = MK(k1,I)
      ENDDO

      GOTO 2520
      END



!     ------------------------------------------------------------------
      DOUBLE PRECISION FUNCTION crms(t)
!     ------------------------------------------------------------------

      USE GlobalArrays
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
!
!     PPU is the vector XO repeated NMS times
!     ppo=vettore XS trasformato da MK(NMA,NMS)
!     t(3) = vettore variazione angolare in radianti
!

      DIMENSION t(3), po(3), pu(3), rpu(3)
      DIMENSION RX(3,3), RY(3,3), RZ(3,3), RR(3,3)

      call vrload(RX,0.d0,9)
      call vrload(RY,0.d0,9)
      call vrload(RZ,0.d0,9)
      call vrload(RR,0.d0,9)

      RX(1,1) =  1.d0
      RX(2,2) =  dcos(t(1))
      RX(2,3) = -dsin(t(1))
      RX(3,2) = -RX(2,3)
      RX(3,3) =  dcos(t(1))
      RY(2,2) =  1.d0
      RY(1,1) =  dcos(t(2))
      RY(1,3) = -dsin(t(2))
      RY(3,1) = -RY(1,3)
      RY(3,3) =  dcos(t(2))
      RZ(1,1) =  dcos(t(3))
      RZ(1,2) = -dsin(t(3))
      RZ(2,1) = -RZ(1,2)
      RZ(2,2) =  dcos(t(3))
      RZ(3,3) =  1.d0

      CALL prodmm(RY,RX,RR,1,1,1)
      CALL prodmm(RZ,RR,RV,1,1,1)
! ortonormalizzazione di precisione
      call prodv(RV,RV,RV,1,2,3)
      call prodv(RV,RV,RV,3,1,2)
      call prodv(RV,RV,RV,2,3,1)
      call norm(RV,1)
      call norm(RV,2)
      call norm(RV,3)
! fine ortonormalizzazione di precisione
      func=0.d0

      DO i=1,npu
         po(1) = ppo(1,i)
         po(2) = ppo(2,i)
         po(3) = ppo(3,i)
         pu(1) = ppu(1,i)
         pu(2) = ppu(2,i)
         pu(3) = ppu(3,i)
         CALL prodmv(RV,pu,pu,1,1,1)
         func = func+(po(1)-pu(1))**2+(po(2)-pu(2))**2+(po(3)-pu(3))**2
      ENDDO

      crms = func
      return
      end



      DOUBLE PRECISION FUNCTION Det(X,N)
      IMPLICIT NONE
      INTEGER N
      DOUBLE PRECISION X(3,3,*)

      Det =    + X(1,1,N)*X(2,2,N)*X(3,3,N) - X(1,1,N)*X(2,3,N)*X(3,2,N)
      Det = Det+ X(1,2,N)*X(2,3,N)*X(3,1,N) - X(1,2,N)*X(2,1,N)*X(3,3,N)
      Det = Det+ X(1,3,N)*X(2,1,N)*X(3,2,N) - X(1,3,N)*X(2,2,N)*X(3,1,N)

      RETURN
      END



      SUBROUTINE eigen(A,VEC,EIG,W,GAM,BET,BSQ,P,Q,IPO,IORD,IVP,NN)
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
!
!     --------------
!     QCPE VERSION
!     DECEMBER 1971
!     --------------
!
!     MATRIX DIAGNOLIZATION ROUTINE FOR REAL SYMMETRIC CASE
!     HOUSEHOLDER METHOD
!     RHO=UPPER LIMIT FOR OFF-DIAGONAL ELEMENT
!     NN SIZE OF MATRIX
!     A=MATRIX (ONLY LOWER TRIANGLE IS USED+THIS IS DESTROYED
!     EIG=RETURNED eigenVALUES IN ALGEBRAIC DESCENDING ORDER
!     VEC=RETURNED eigenVECTORS IN COLUMNS
!
!
      DIMENSION A(NN,NN), VEC(NN,NN), EIG(NN), W(NN), GAM(NN), BET(NN)
      DIMENSION BSQ(NN), P(NN), Q(NN), IPO(NN), IORD(NN), IVP(NN)
 
      DATA RHOSQ /1.0D-12/
      ADUE=.50D0
      ZERO=0.D0
      UNO=1.0D0
      DUE=2.0D0
 
      N=NN
      IF(N)10,550,10
   10 N1=N-1
      N2=N-2
      GAM(1)=A(1,1)
      IF(N2) 180,170,20
   20 DO 160 NR=1,N2
      B=A(NR+1,NR)
      S=ZERO
      DO 30 I=NR,N2
   30 S=S+A(I+2,NR)**2
!     PREPARE FOR POSSIBLE BYPASS OF TRANSFORMATION
      A(NR+1,NR)=ZERO
      IF (S) 150,150,40

   40 S=S+B*B
      SGN=UNO
      IF (B) 50,60,60

   50 SGN=-UNO
   60 SQRTS=SQRT(S)
      D=SGN/(SQRTS+SQRTS)
      TEMP=SQRT(ADUE+B*D)
      W(NR)=TEMP
      A(NR+1,NR)=TEMP
      D=D/TEMP
      B=-SGN*SQRTS
!     D IS FACTOR OF PROPORTIONALITY. NOW COMPUTE AND SAVE W VECTOR.
!     EXTRA SINGLY SUBSCRIPTED W VECTOR USED FOR SPEED.
      DO 70 I=NR,N2
      TEMP=D*A(I+2,NR)
      W(I+1)=TEMP
   70 A(I+2,NR)=TEMP
!     PREMULTIPLY VECTOR W BY MATRIX A TO OBTAIN P VECTOR.
!     SIMULTANEOUSLY ACCUMULATE DOT PRODUCT WP,(THE SCALAR K)
      WTAW=ZERO
      DO 120 I=NR,N1
      I1=I+1
      SUM=ZERO
      DO 80 J=NR,I
   80 SUM=SUM+A(I1 ,J+1)*W(J)
      IF(N1-I1) 110,90,90
   90 DO 100 J=I1,N1
  100 SUM=SUM+A(J+1,I1 )*W(J)
  110 P(I)=SUM
      WWWI=W(I)
  120 WTAW=WTAW+SUM*WWWI
!     P VECTOR AND SCALAR K  NOW STORED. NEXT COMPUTE Q VECTOR
      DO 130 I=NR,N1
  130 Q(I)=P(I)-WTAW*W(I)
!     NOW FORM PAP MATRIX, REQUIRED PART
      DO 140 J=NR,N1
      QJ=Q(J)
      WJ=W(J)
      DO 140 I=J,N1
  140 A(I+1,J+1)=A(I+1,J+1)-DUE*(W(I)*QJ+WJ*Q(I))
  150 BET(NR)=B
      BSQ(NR)=B*B
  160 GAM(NR+1)=A(NR+1,NR+1)
  170 B=A(N,N1)
      BET(N1)=B
      BSQ(N1)=B*B
      GAM(N)=A(N,N)
  180 BSQ(N)=ZERO
!     ADJOIN AN IDENTIFY MATRIX TO BE POSTMULTIPLIED BY ROTATIONS.
      DO 200 I=1,N
      DO 190 J=1,N
  190 VEC(I,J)=ZERO
  200 VEC(I,I)=UNO
      M=N
      SUM=ZERO
      NPAS=1
      GO TO 330
  210 SUM=SUM+SHIFT
      COSA=UNO
      G=GAM(1)-SHIFT
      PP=G
      PPBS=PP*PP+BSQ(1)
      PPBR=SQRT(PPBS)
      DO 300 J=1,M
      COSAP=COSA
      IF(PPBS)230,220,230
  220 SINA=ZERO
      SINA2=ZERO
      COSA=UNO
      GO TO 270
  230 SINA=BET(J)/PPBR
      SINA2=BSQ(J)/PPBS
      COSA=PP/PPBR
!     POSTMULTIPLY IDENTITY BY P-TRANSPOSE MATRIX
      NT=J+NPAS
      IF(NT-N)250,240,240
  240 NT=N
  250 DO 260 I=1,NT
      TEMP=COSA*VEC(I,J)+SINA*VEC(I,J+1)
      VEC(I,J+1)=-SINA*VEC(I,J)+COSA*VEC(I,J+1)
  260 VEC(I,J)=TEMP
  270 DIA=GAM(J+1)-SHIFT
      U=SINA2*(G+DIA)
      GAM(J)=G+U
      G=DIA-U
      PP=DIA*COSA-SINA*COSAP*BET(J)
      IF(J-M)290,280,290
  280 BET(J)=SINA*PP
      BSQ(J)=SINA2*PP*PP
      GO TO 310
  290 PPBS=PP*PP+BSQ(J+1)
      PPBR=SQRT(PPBS)
      BET(J)=SINA*PPBR
  300 BSQ(J)=SINA2*PPBS
  310 GAM(M+1)=G
!     TEST FOR CONVERGENCE OF LAST DIAGONAL ELEMENT
      NPAS=NPAS+1
      IF(BSQ(M)-RHOSQ)320,320,350
  320 EIG(M+1)=GAM(M+1)+SUM
  330 BET(M)=ZERO
      BSQ(M)=ZERO
      M=M-1
      IF(M)340,380,340
  340 IF(BSQ(M)-RHOSQ)320,320,350
!     TAKE ROOT OF CORNER 2 BY 2 NEAREST TO LOWER DIAGONAL IN VALUE
!     AS ESTIMATE OF eigenVALUE TO USE FOR SHIFT
  350 A2=GAM(M+1)
      R2=ADUE*A2
      R1=ADUE*GAM(M)
      R12=R1+R2
      DIF=R1-R2
      TEMP=SQRT(DIF*DIF+BSQ(M))
      R1=R12+TEMP
      R2=R12-TEMP
      DIF=ABS(A2-R1)-ABS(A2-R2)
      IF(DIF)370,360,360
  360 SHIFT=R2
      GO TO 210
  370 SHIFT=R1
      GO TO 210
  380 EIG(1)=GAM(1)+SUM
!     INITIALIZE AUXILIARY TABLES REQUIRED FOR REARRANGING THE VECTORS
      DO 390 J=1,N
      IPO(J)=J
      IVP(J)=J
  390 IORD(J)=J
!     USE A TRANSPOSITION SORT TO ORDER THE eigenVALUES
      M=N
      GO TO 430
  400 DO 420 J=1,M
      IF(EIG(J)-EIG(J+1))410,420,420
  410 TEMP=EIG(J)
      EIG(J)=EIG(J+1)
      EIG(J+1)=TEMP
      ITEMP=IORD(J)
      IORD(J)=IORD(J+1)
      IORD(J+1)=ITEMP
  420 CONTINUE
  430 M=M-1
      IF(M)400,440,400
  440 IF(N1)450,490,450
  450 DO 480 L=1,N1
      NV=IORD(L)
      NP=IPO(NV)
      IF(NP-L)460,480,460
  460 LV=IVP(L)
      IVP(NP)=LV
      IPO(LV)=NP
      DO 470 I=1,N
      TEMP=VEC(I,L)
      VEC(I,L)=VEC(I,NP)
  470 VEC(I,NP)=TEMP
  480 CONTINUE
!     BACK TRANSFORM THE VECTORS OF THE TRIPLE DIAGONAL MATRIX
  490 DO 540 NRR=1,N
      K=N1
  500 K=K-1
      IF(K)540,540,510
  510 SUM=ZERO
      DO 520 I=K,N1
  520 SUM=SUM+VEC(I+1,NRR)*A(I+1,K)
      SUM=SUM+SUM
      DO 530 I=K,N1
  530 VEC(I+1,NRR)=VEC(I+1,NRR)-SUM*A(I+1,K)
      GO TO 500
  540 CONTINUE
  550 RETURN
      END



!     ------------------------------------------------------------------
      SUBROUTINE eq_plane(t,u,v,a)
!     ------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION t(3), u(3), v(3), AA(3,3), DD(3,3), a(4)

!     Equazione del piano passante per i punti t,u,v nella forma canonica
!     a(1).x+a(2).y+a(3).z+a(4)=0 
!     con a(1),a(2),a(3)=coseni direttori a(4)=-distanza piano-origine
!     vedi International Tables for Crystallography II, p. 43, equaz. 2,3,4, 6

      DD(1,1) = t(1)
      DD(1,2) = t(2)
      DD(1,3) = t(3)
      DD(2,1) = u(1)
      DD(2,2) = u(2)
      DD(2,3) = u(3)
      DD(3,1) = v(1)
      DD(3,2) = v(2)
      DD(3,3) = v(3)

      DO i = 1,3
         CALL MatrixCopy(DD,AA,1,1)
         CALL vrload(AA(1,i),1.d0,3)
         a(i)=det(AA,1)
      ENDDO

      CALL prods(a,a,dist,1,1,2)

      a(1) = a(1)/dist
      a(2) = a(2)/dist
      a(3) = a(3)/dist
      a(4) = -det(DD,1)/dist

      RETURN
      END



!     ------------------------------------------------------------------
      SUBROUTINE icosahed(XO,PESO,N,MN,MK,MD,II,MDEG,*)
!     ------------------------------------------------------------------

      USE GlobalArrays
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION XO(3,NMA), PESO(NMA), MN(NMA), MK(NMA,NMG), MD(NMA,2)
      DIMENSION eqp(4), dp(5), vd(3,5), A(3,3), B(3,3), V(3)
      DIMENSION io(5), mp(5)
      LOGICAL   ico

      REAL*8,  DIMENSION (:), ALLOCATABLE :: DA
      INTEGER, DIMENSION (:), ALLOCATABLE :: meq

      ALLOCATE (da(nma))
      ALLOCATE (meq(nma))

      RAD = 57.29577951308232D0

      DO 2000 I1 = 1,N-4
         IF (MD(I1,1).ne.II) GOTO 2000
         mp(1) = I1
         io(1) = I1
         in1   = I1+1
         DO 1900 I2=in1,N-3
            IF (MD(I1,1).ne.II) GOTO 1900
            mp(2) = I2
            in2   = I2+1
            DO 1800 I3 = in2,N-2
               IF (MD(I3,1).ne.II) GOTO 1800
               mp(3)=I3
               call eq_plane(XO(1,I1),XO(1,I2),XO(1,I3),eqp)
!              se il piano e' troppo vicino all'origine viene scartato
              if (eqp(4).lt.0.5) GOTO 1800
              in3=I3+1
              do 1700 I4=in3,N-1
                 if(MD(I4,1).ne.II) GOTO 1700
                 mp(4)=I4
                 d2=eqp(1)*XO(1,I4)+eqp(2)*XO(2,I4)+eqp(3)*XO(3,I4)+eqp(4)
                 if (DABS(d2).gt.DXM(I4)) GOTO 1700
 1300            in4=I4+1
                 do 1600 I5 = in4,N
                    if (MD(I5,1).ne.II) GOTO 1600
                    mp(5)=I5
                    d2=eqp(1)*XO(1,I5)+eqp(2)*XO(2,I5)+eqp(3)*XO(3,I5)+eqp(4)
                    if(DABS(d2).gt.DXM(I5))go to 1600
! in mp ci sono i possibili 5 atomi equivalenti rispetto all'asse 5
      dmin=100.
      dp(1)=dmin
      do i=2,5
        io(i)=0
        dp(i)=0.d0
        call LinearCombination(XO,XO,vd,1.d0,-1.d0,I1,mp(i),1)
        call prods(vd,vd,dp(i),1,1,2)
        if(dp(i).lt.dmin)then
          dmin=dp(i)
          jj=i
        endif
      enddo
      io(2)=mp(jj)
      dp(jj)=100.
      dmin=100.
      do i=2,5
        if(dp(i).lt.dmin.and.io(2).ne.mp(i))then
          dmin=dp(i)
          kk=i
        endif
      enddo 
      io(5)=mp(kk)
      do i=2,5
      if(mp(i).ne.io(2).and.mp(i).ne.io(5).and.io(3).eq.0)io(3)=mp(i)
      if(mp(i).ne.io(2).and.mp(i).ne.io(5).and.mp(i).ne.io(3))jj=mp(i)
      enddo
      call LinearCombination(XO,XO,vd,1.d0,-1.d0,io(2),io(3),1)
      call prods(vd,vd,d1,1,1,1)
      call LinearCombination(XO,XO,vd,1.d0,-1.d0,io(2),jj,1)
      call prods(vd,vd,d2,1,1,1)
      io(4)=jj
      if(d2.lt.d1)then
      io(4)=io(3)
      io(3)=jj
      endif
! ora i cinque atomi sono ordinati
! per controllo preliminare prima di passare alla verify, controllo 
!he siano uguali ( a meno della tolleranza) le distanze 1-2
      dmed=0.
      do i=1,5
        k=i+1
        if(k.gt.5)k=k-5
        call LinearCombination(XO,XO,vd,1.d0,-1.d0,io(i),io(k),i)
        call prods(vd,vd,dp(i),i,i,2)
        dmed=dmed+dp(i)
      enddo
      dmed=dmed*0.2d0
!     write(out,'(a5,5i5)')'io',io
!     write(out,'(a5,5f10.5)')'dp',dp
!     write(out,'(a5,5f10.5)')'dmed',dmed
      call vrload(A,0.d0,9)
      do i=1,5
        if(DABS(dmed-dp(i)).gt.DXM(i)*.5)go to 1600
! asse C5 come somma dei vertici del pentagono
        A(1,3)=A(1,3)+XO(1,io(i))
        A(2,3)=A(2,3)+XO(2,io(i))
        A(3,3)=A(3,3)+XO(3,io(i))
      enddo
!     write(out,'(a5,3f10.6)')'A(3)',(A(i,3),i=1,3)
! mette il riferimento in modo che il primo asse 5 coincida con z e che
! il secondo sia nel piano xz
      A(1,1) =XO(1,io(1))+XO(1,io(2))-XO(1,io(3))-XO(1,io(4))+XO(1,io(5))
      A(2,1) =XO(2,io(1))+XO(2,io(2))-XO(2,io(3))-XO(2,io(4))+XO(2,io(5))
      A(3,1) =XO(3,io(1))+XO(3,io(2))-XO(3,io(3))-XO(3,io(4))+XO(3,io(5))
! se la somma dei cinque atomi da' un vettore nullo, cioe' il pentagono
! e' sul cerchio massimo (caso possibile per un gruppo di 30 atomi sugli
! assi C2) passo ad altro pentagono
 1400 call norm(A,3)
      call norm(A,1)
      call prodv(A,A,A,3,1,2)
      call norm(A,2)
      call prodv(A,A,A,2,3,1)
      call TransposeMatrix(A,A,1,1)
!     write(out,'(a5,3(/,3f10.6))')'A',((A(i,k),i=1,3),k=1,3)
      call RotateFrame(A,XO,N)
!ostruzione dell'asse C5
      call vrload(B,0.d0,9)
      ROT=72.d0/RAD
      CA=COS(ROT)
      CB=SIN(ROT)
      B(1,1)=CA
      B(2,2)=CA
      B(1,2)=-CB
      B(2,1)=CB
      B(3,3)=1.d0
!     write(out,'(a5,3(/,3f10.6))')'B',((B(i,k),i=1,3),k=1,3)
! verifica e ottimizza il primo asse 5
      call verify(XO,B,MK,MN,MV,N)
!     write(out,*)'primo asse C5 MV,NMS',MV,NMS
!     write(out,'(30i3)')(MK(ll,NMS),ll=1,N)
      if(MV.eq.0)go to 1600
      call opt_axis(XO,PESO,V,MK,N,2)
! ottimizza l'sse x con la stessa tecnica precedente
      call vrload(A,0.d0,9)
      if(V(3).lt.0)then
      V(1)=-V(1)
      V(2)=-V(2)
      V(3)=-V(3)
      endif
      call VectorCopy(V,A,1,3)
      A(1,1)=XO(1,io(1))+XO(1,io(2))-XO(1,io(3))-XO(1,io(4))+XO(1,io(5))
      A(2,1)=XO(2,io(1))+XO(2,io(2))-XO(2,io(3))-XO(2,io(4))+XO(2,io(5))
      A(3,1)=XO(3,io(1))+XO(3,io(2))-XO(3,io(3))-XO(3,io(4))+XO(3,io(5))
 1500 call norm(A,3)
      call norm(A,1)
      call prodv(A,A,A,3,1,2)
      call norm(A,2)
      call prodv(A,A,A,2,3,1)
      call TransposeMatrix(A,A,1,1)
!     write(out,'(a5,3(/,3f10.6))')'A',((A(i,k),i=1,3),k=1,3)
      call RotateFrame(A,XO,N)
      go to 2010
 1600 continue
 1700 continue
 1800 continue
 1900 continue
 2000 continue

!     uscita normale (non ha trovato nessun asse C5)
      IF (NMS.eq.1) THEN
         DEALLOCATE (da, meq)
         RETURN
      ENDIF

!ompleta il gruppo C5. Indispensabile qui!!!
 2010 call CompleteGroup(MK,N,*2015)
! scelgo i due pentagoni piu' distanti dall'asse C5 per trovare
!  un asse C2. nel caso che gli atomi del sottoset non siano
! tutti indipendenti, postrebbero esserci piu' di cinque atomi
! sul piano parallelo al primo pentagono e quindi devo 
! testare tutti i possibili pentagoni
 2015 DM=0.d0
      do 2020 i=1,N
      if(MD(I1,1).ne.II)go to 2020
      DA(i)=XO(1,I)**2+XO(2,I)**2
      if(DM.gt.DA(i))go to 2020
      kk=i
      DM=DA(i)
 2020 continue
      io(1)=kk
      dp1=XO(3,kk)
!     write(out,*)'io(1),dp1',io(1),dp1
      do k=1,4
        k1=k+1
        io(k1)=MK(io(k),2)
        dp1=dp1+XO(3,io(k1))
      enddo
      dp1=-0.2d0*dp1
! in io(i) ci sono i cinque atomi del primo pentagono
! in meq(i) vanno gli atomi del piano parallelo
      me=0
      do  2030 i=1,N
      com=DABS(dp1-XO(3,i))
      if(com.gt.DXM(i))go to 2030
! eliminazione (eventuale) degli atomi del primo piano
      do k=1,5
        if(i.eq.io(k))go to 2030
      enddo
      me=me+1
      meq(me)=i
 2030 continue
!     write(out,*)'me,meq',me
!     write(out,'(30i4)')(meq(i),i=1,me)
! scelta degli atomi del secondo pentagono
 2040 do i=1,me
        if(meq(i).ne.0)go to 2050
      enddo
! non ci sono assi C2!!!
      MDEG=1

      DEALLOCATE (da, meq)
      RETURN

!     i e' il primo atomo del nuovo pentagono
 2050 mp(1)  = meq(i)
      meq(i) = 0
      k = 1

!     cerca gli altri atomi del secondo penatgono e annulla i relativi meq
      DO k = 1,4
         k1=k+1
         mp(k1) = MK(mp(k),2)
         DO l = 1,me
            IF (mp(k1).eq.meq(l)) meq(l) = 0
         ENDDO
      ENDDO

! somma vettori del primo pentagono opportunamente ruotati
! costruzione dell'asse C2 perpendicolare a C5 e parallelo a x
 2210 call vrload(A,0.d0,9)
      call vrload(vd,0.d0,15)
!     write(out,*)'io,mp'
!     write(out,'(5i4)')io,mp
      do 2250 i=1,5
      k=io(i)
      do 2220 l=1,NMS
      if(MK(k,l).ne.io(1))go to 2220
      call prodmv(SIM,XO,vd,l,k,3)
      call LinearCombination(vd,vd,vd,1.d0,1.d0,3,1,1)
!     write(out,'(a20,i5,3f10.5)')'io(i),vd(1)',io(i),(vd(kk,1),kk=1,3)
      go to 2230
 2220 continue
 2230 k=mp(i)
      do 2240 l=1,NMS
      if(MK(k,l).ne.mp(1))go to 2240
      call prodmv(SIM,XO,vd,l,k,3)
      call LinearCombination(vd,vd,vd,1.d0,1.d0,3,2,2)
!     write(out,'(a20,i5,3f10.5)')'mp(i),vd(2)',mp(i),(vd(kk,2),kk=1,3)
      go to 2250
 2240 continue
 2250 continue

      vd(3,1)=0.d0
      vd(3,2)=0.d0
      call norm(vd,1)
      call norm(vd,2)
!     write(out,'(a20,3f10.5)')'vd(1)',(vd(kk,1),kk=1,3)
!     write(out,'(a20,3f10.5)')'vd(2)',(vd(kk,2),kk=1,3)
      cost=DCOS(36.1/RAD)
      do i=1,5
        call prodmv(SIM,vd,vd,2,2,2)
        call prods(vd,vd,cosa,1,2,1)
!     write(out,'(a14,8f9.5)')'vd(1),vd(2)',(vd(kk,1),kk=1,3),
!    *(vd(kk,2),kk=1,3),cosa,cost
        if(cosa.ge.cost)go to 2300
      enddo
 2300 call LinearCombination(vd,vd,A,1.d0,1.d0,1,2,1)
      call norm(A,1)
!     write(out,'(a20,3f10.5)') 'A(1)',(A(kk,1),kk=1,3)
      A(3,3)=1.d0
      call prodv(A,A,A,3,1,2)
      call norm(A,2)
      call prodv(A,A,A,2,3,1)
      call norm(A,1)
      call TransposeMatrix(A,A,1,1)
      call RotateFrame(A,XO,N)
!     write(out,'(i4,3f9.5)')(lll,(XO(kk,lll),kk=1,3),lll=1,N)
      call vrload(B,0.d0,9)
      B(1,1)= 1.d0
      B(2,2)=-1.d0
      B(3,3)=-1.d0
      call verify(XO,B,MK,MN,MV,N)
!     write(out,*)'asse C2 MV,NMS',MV,NMS
      if(MV.eq.1)go to 2310
      call TransposeMatrix(A,A,1,1)
      call RotateFrame(A,XO,N)
      call vrload(A,0.d0,9)
      A(1,2)=-1.d0
      A(2,1)= 1.d0
      A(3,3)= 1.d0
      call RotateFrame(A,XO,N)
      call verify(XO,B,MK,MN,MV,N)
!     write(out,*)'asse C2 MV,NMS',MV,NMS
      if(MV.eq.1)go to 2310
      call TransposeMatrix(A,A,1,1)
      call RotateFrame(A,XO,N)
      go to 2040
!     ico=.true.
!     call CompleteGroup(MK,N,*5000)
!     write(out,*)'asse C2,NMS',NMS
!     ontrolla il centro

 2310 call vrload(B,0.d0,9)
      B(1,1)=-1.d0
      B(2,2)=-1.d0
      B(3,3)=-1.d0
      call verify(XO,B,MK,MN,MV,N)

!     write(out,*)'CENTRO MV,NMS',MV,NMS
!ostruzione dell'asse C5 a |x|=63.43494882 da z e la cui priezione e' a 18 da x
      call vrload(A,0.d0,9)
      ROT=-63.43494882d0/RAD
 3000 CA=COS(ROT)
      CB=SIN(ROT)
      call vrload(A,0.d0,9)
      A(1,1)=1.d0
      A(2,2)= CA
      A(3,3)= CA
      A(2,3)=-CB
      A(3,2)= CB
! rotazione 18 su z
      call vrload(B,0.d0,9)
      ROT18=-18.d0/RAD
 3100 CA=COS(ROT18)
      CB=SIN(ROT18)
      call vrload(B,0.d0,9)
      B(1,1)= CA
      B(2,2)= CA
      B(1,2)=-CB
      B(2,1)= CB
      B(3,3)=1.d0
      call prodmm(B,A,A,1,1,1)
!     write(out,*)' matrice di rotazione'
!     write(out,'(a12,2f10.6)')'ROT,ROT18',ROT,ROT18
!     write(out,'(3f10.6)')((A(i,k),k=1,3),i=1,3)
      call prodmm(SIM,A,B,2,1,1)
      call TransposeMatrix(A,A,1,1)
      call prodmm(A,B,A,1,1,1)
      call verify(XO,A,MK,MN,MV,N)
!     write(out,*)'secondo asse C5 ,MV,NMS',MV,NMS
      if(MV.eq.1)go to 4000
      if(ROT18.lt.0.d0)then
      ROT18=-ROT18
      go to 3100
      endif

      IF (ROT.gt.0.d0) THEN
         DEALLOCATE (da, meq)
         RETURN
      ENDIF
     
      ROT = -ROT
      GOTO 3000

 4000 CALL CompleteGroup(MK,N,*5000)

      DEALLOCATE (da, meq)
 5000 RETURN 1

      END



      SUBROUTINE intpe(RIGA,B,C,IA,NB,*,*)
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      CHARACTER*(*) RIGA
      CHARACTER*23 CAR, CARA

!     INTERPRETA IL VETTORE  A COME POSIZIONE EQUIVALENTE O
!     PREPARA LO STESSO PER LA STAMPA A PARTIRE DA B E C
      DIMENSION B(3,3,*), C(3,*)

      CAR='1234567890 /,+-xyz[]XYZ'

      GOTO (10,500),NB
! INTERPRETAZIONE DI A
   10 MCM=1
      LR=LEN(RIGA)
      call compatta(RIGA,LR,KM)
      K=1
      COST=1.d0
      DO 15 I=1,3
      C(I,IA)=0.d0
      DO 15 J=1,3
   15 B(I,J,IA)=0.d0
      DO 200 I=1,KM
! IDENTIFICAZIONE DEL CARATTERE
      CARA=RIGA(I:I)
      DO 20 J=1,23
      IF(CAR(J:J).EQ.CARA)GO TO 30
   20 CONTINUE
   25 return 2
   30 IF(J-11)40,200,100
! CARATTERE NUMERICO
   40 IF(J.EQ.10)J=0
      GO TO(50,60),MCM
! NUMERATORE
   50 IF(C(K,IA).NE.0.d0)C(K,IA)=C(K,IA)*10
      C(K,IA)=C(K,IA)+COST*J
      COST=1.d0
      GO TO 200
! DENOMINATORE
   60 C(K,IA)=C(K,IA)/J
      MCM=1
      GO TO 200
!ARATTERI ALFABETICI
  100 if(J.gt.20)J=J-5
      J=J-11
      GO TO(110,120,130,140,150,150,150,200,210),J
! BARRA
  110 MCM=2
      GO TO 200
!AMBIO RIGA MATRICE (VIRGOLA)
  120 K=K+1
      GO TO 200
! SEGNO +
  130 COST=1.d0
      GO TO 200
! SEGNO -
  140 COST=-1.d0
      GO TO 200
!  X,Y O Z
  150 J=J-4
      B(K,J,IA)=COST
      COST=1.d0
  200 CONTINUE
  210 RETURN 1
! SCRITTURA  DEL VETTORE POSIZIONE EQUIVALENTE ALFANUMERICA
  500 KM=LEN(RIGA)
      DO 510 I=1,KM
  510 RIGA(i:i)=car(11:11)
      RIGA(12:12)=','
      RIGA(23:23)=','
      DO 1000 I=1,3
      MCM=11*I
      DO 600 J=1,3
      L=4-J
      IF(NINT(B(I,L,IA)).EQ.0)GO TO 600
      MA=L+15
      RIGA(MCM:MCM)=CAR(MA:MA)
      K=14
      IF(B(I,L,IA).LT.(-0.1d0))K=15
      MCM=MCM-1
      RIGA(MCM:MCM)=CAR(K:K)
      MCM=MCM-1
  600 CONTINUE
      MS=11
      IF(ABS(C(I,IA)).LT.0.1d0)GO TO 1000
      IF(C(I,IA).LT.0.d0)MS=15
      DO 700 J=1,6
      CA=C(I,IA)*J
      K=NINT(CA)
      IF(ABS(FLOAT(K)-CA).LT.0.1d0)GO TO 800
  700 CONTINUE
  800 IF(J.EQ.1)GO TO 900
      RIGA(MCM:MCM)=CAR(J:J)
      MCM=MCM-1
      RIGA(MCM:MCM)=CAR(12:12)
      MCM=MCM-1
  900 J=IABS(MOD(K,10))
      RIGA(MCM:MCM)=CAR(J:J)
      K=K/10
      MCM=MCM-1
      IF(K.GT.0)GO TO 900
      RIGA(MCM:MCM)=CAR(MS:MS)
 1000 CONTINUE
      KM1=KM-1
      call compatta(RIGA(2:KM),KM1,K)
      if(RIGA(2:2).eq.'+')RIGA(2:2)=' '
      do 1010 i=1,2
      k=index(RIGA,',+')
      k1=k+1
      if(k.ne.0)RIGA(k1:k1)=' '
 1010 continue
      call compatta(RIGA(2:KM),KM1,K)
      RETURN 1
      END



!     Inversion of a 3x3 matrix
!     ------------------------------------------------------------------
      SUBROUTINE Invert3x3(X,Y,K,L)
!     ------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION X(3,3,1),Y(3,3,1)

      C = 1.0/det(X,K)
      Y(1,1,L) = (X(2,2,K)*X(3,3,K) - X(2,3,K)*X(3,2,K))*C
      Y(2,1,L) = (X(2,3,K)*X(3,1,K) - X(2,1,K)*X(3,3,K))*C
      Y(3,1,L) = (X(2,1,K)*X(3,2,K) - X(2,2,K)*X(3,1,K))*C
      Y(1,2,L) = (X(1,3,K)*X(3,2,K) - X(1,2,K)*X(3,3,K))*C
      Y(2,2,L) = (X(1,1,K)*X(3,3,K) - X(1,3,K)*X(3,1,K))*C
      Y(3,2,L) = (X(1,2,K)*X(3,1,K) - X(1,1,K)*X(3,2,K))*C
      Y(1,3,L) = (X(1,2,K)*X(2,3,K) - X(1,3,K)*X(2,2,K))*C
      Y(2,3,L) = (X(1,3,K)*X(2,1,K) - X(1,1,K)*X(2,3,K))*C
      Y(3,3,L) = (X(1,1,K)*X(2,2,K) - X(1,2,K)*X(2,1,K))*C

      RETURN
      END



      FUNCTION ium(A,B,C,M,N)
!     Returns if (A-B) < C
!     ONTROLLA  SE A=B A MENO DI C
!     True ium=1      False  ium=0
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,3,*), B(3,3,*)
 
      ium = 0
      DO I = 1,3
         DO J = 1,3
            IF(ABS(A(I,J,M)-B(I,J,N)).GT.C) RETURN
         ENDDO
      ENDDO
      ium = 1

      RETURN
      END


      SUBROUTINE MassData(atomicNumbers)
      USE GlobalArrays
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER (MaxAtomicNumber = 103)
      INTEGER   atomicNumbers(nAtoms), Z, ITT
      CHARACTER*2 SIMBO(MaxAtomicNumber)
      REAL*8 AtomicMass(MaxAtomicNumber), Radii(MaxAtomicNumber)

      DATA (AtomicMass(i), i=1,2)  /1.00794, 4.002602/
      DATA (AtomicMass(i), i=3,10) /6.941, 9.012182, 10.811, 12.011,    &
         14.00674, 15.9994, 18.9984032, 20.1797/
      DATA (AtomicMass(i), i=11,18) /22.989768, 24.3050, 26.981539,     &
         28.0855, 30.97362, 32.066, 35.4527, 39.948/
      DATA (AtomicMass(i), i=19,36) /39.0983, 40.078, 44.955910, 47.88, &
         50.9415, 51.9961, 54.93085, 55.847, 58.93320, 58.69, 63.546,   &
         65.39, 69.723, 72.61, 74.92159, 78.96, 79.904, 83.80/
      DATA (AtomicMass(i), i=37,54)/85.4678,87.62,88.90585,91.224,      &
         92.90638,95.94,98,101.07,102.90550,106.42,107.8682,112.411,    &
         114.82,118.710,121.75,127.60,126.90447,131.29/
      DATA (AtomicMass(i), i=55,86)/132.90543,137.327,138.9055,140.115, &
         140.90765,144.24,145,150.36,151.965,157.25,158.92534,162.50,   &
         164.93032,167.26,168.93421,173.04,174.967,178.49,180.9479,     &
         183.85,186.207,190.2,192.22,195.08,196.96654,200.59,204.3833,  &
         207.2,208.98037,209,210.0,222/
      DATA (AtomicMass(i), i=87,103)/223,226.025,227.028,232.0381,      &
         231.03588,238.0289,237.048,244.,243.,247.,247.,251.,252.,      &
         257.,258.,259.,260.0/


      DATA (SIMBO(i),i = 1,2)/' H','HE'/
      DATA (SIMBO(i),i = 3,10)/'LI','BE',' B',' C',' N',' O',' F','NE'/
      DATA (SIMBO(i),i = 11,18)/'NA','MG','AL','SI',' P',' S','CL','AR'/
      DATA (SIMBO(i),i = 19,36)/' K','CA','SC','TI',' V','CR','MN','FE', &
      'CO','NI','CU','ZN','GA','GE','AS','SE','BR','KR'/
      DATA (SIMBO(i),i = 37,54)/'RB','SR',' Y','ZR','Nb','MO','TC','RU', &
      'RH','PD','AG','CD','IN','SN','SB','TE',' I','XE'/
      DATA (SIMBO(i),i = 55,86)/'CS','BA','LA','CE','PR','ND','PM', &
      'SM','EU','GD','TB','DY','HO','ER','TM','YB','LU','HF','TA',' W', &
      'RE','OS','IR','PT','AU','HG','TL','PB','BI','PO','AT','RN'/
      DATA (SIMBO(i),i = 87,103)/'FR','RA','AC','TH','PA',' U','NP', &
      'PU','AM','CM','BK','CF','ES','FM','MD','NO','LR'/

      DO J = 1,nAtoms
         Z = atomicNumbers(j)
         AMAS(j) = AtomicMass(Z)
      ENDDO

      DO J = 1,nAtoms
         MSP(J) = 0
      ENDDO

      M = 1
      ITT = atomicNumbers(1)
 10   DO K = 1,nAtoms
         IF (atomicNumbers(K) .EQ. ITT) MSP(K) = M
      ENDDO
      M = M+1

      DO j = 1,nAtoms
         IF (MSP(J).EQ.0) THEN
            ITT = atomicNumbers(j)
            MSP(J) = M
            GOTO 10
         ENDIF
      ENDDO

      RETURN
      END



      SUBROUTINE momin(XO,P,RO,N)

      USE GlobalArrays

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
! da un set di coordinate XO che vengono moltiplicate per la matrice RO
! che puo' essere una matrice di ortogonalizzazione calcola i momenti di
! inerzia utilizzando i pesi P

      DIMENSION RO(3,3), AA(3,3), AMOM(6), CAM(3), ORI(3,3)
      DIMENSION C1(3), C2(3), C3(3), C4(3), C5(3), C6(3)
      DIMENSION IC1(3), IC2(3), IC3(3)
      DIMENSION XO(3,*), P(*)

      DO 100 I=1,3
      RIN(I)=0.d0
      AMOM(I)=0.d0
  100 AMOM(I+3)=0.d0
      SUM=0.d0
      DO 110 I=1,N
      call prodmv(RO,XO,XO,1,I,I)

      PESO=P(I)
      RIN(1)=RIN(1)+PESO*XO(1,I)
      RIN(2)=RIN(2)+PESO*XO(2,I)
      RIN(3)=RIN(3)+PESO*XO(3,I)
  110 SUM=SUM+PESO
      DO 115 I=1,3
      RIN(I)=RIN(I)/SUM
  115 BARO(I)=RIN(I)
      DO 120 I=1,N
      PESO=P(I)
      XO(1,I)=XO(1,I)-BARO(1)
      XO(2,I)=XO(2,I)-BARO(2)
      XO(3,I)=XO(3,I)-BARO(3)
      CAM(1)=XO(1,I)*PESO
      CAM(2)=XO(2,I)*PESO
      CAM(3)=XO(3,I)*PESO
      AMOM(1)=AMOM(1)+CAM(1)*XO(1,I)
      AMOM(2)=AMOM(2)-CAM(1)*XO(2,I)
      AMOM(3)=AMOM(3)-CAM(1)*XO(3,I)
      AMOM(4)=AMOM(4)+CAM(2)*XO(2,I)
      AMOM(5)=AMOM(5)-CAM(2)*XO(3,I)
      AMOM(6)=AMOM(6)+CAM(3)*XO(3,I)
  120 CONTINUE
      AA(1,1)=AMOM(4)+AMOM(6)
      AA(2,2)=AMOM(1)+AMOM(6)
      AA(3,3)=AMOM(1)+AMOM(4)
      AA(1,2)=AMOM(2)
      AA(2,1)=AMOM(2)
      AA(1,3)=AMOM(3)
      AA(3,1)=AMOM(3)
      AA(2,3)=AMOM(5)
      AA(3,2)=AMOM(5)
      call eigen(AA,OR,RIN,C1,C2,C3,C4,C5,C6,IC1,IC2,IC3,3)
!__________________
! modifica per rendere OR machine-independent
      am1=-1
      am2=-1
      do i=1,3
        if(DABS(OR(i,1)).gt.am1)then
          am1=DABS(OR(i,1))
          n1=i
        endif
        if(DABS(OR(i,2)).gt.am2)then
          am2=DABS(OR(i,2))
          n2=i
        endif
      enddo
      if(OR(n1,1).lt.0.0)then
        do i=1,3
          OR(i,1)=-OR(i,1)
        enddo
      endif
      if(OR(n2,1).lt.0.0)then
        do i=1,3
          OR(i,2)=-OR(i,2)
        enddo
      endif
      CALL prodv(OR,OR,OR,1,2,3)
!__________________
      call TransposeMatrix(OR,OR,1,1)
      call prodmm(OR,RO,OT,1,1,1)
      call Invert3x3(OT,OTI,1,1)
      call Invert3x3(RO,ORI,1,1)
      call prodmv(ORI,BARO,BARC,1,1,1)
      do 130 i=1,N
  130 call prodmv(OR,XO,XO,1,I,I)
      RETURN
      END




!     ------------------------------------------------------------------
      SUBROUTINE norm(A,I)
!     ------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,*)

      CALL prods(A,A,B,I,I,2)

      DO J = 1,3
         A(J,I) = A(J,I)/B
      ENDDO

      RETURN
      END



      subroutine opt_axis(XO,PESO,V3,MK,N,IAX)
      USE GlobalArrays

      implicit double precision (a-h,o-z)

      DIMENSION XO(3,NMA),MK(NMA,NMG),V3(3),PESO(NMA),VS(3)
      DIMENSION IV(NMA),CO(3,NMA),W(NMA),nge(NMA)
! it makes so that the sum of the distances dall' element of symmetry IAX is minimal
! fa in modo che gli la somma delle distanze dall'elemento di simmetria IAX
! sia minimo
      do i=1,N
        iv(i)=0
      enddo
      ng=0
      cost=det(SIM,IAX)
 1000 do I=1,N
         if(iv(i).eq.0)go to 1100
      enddo
      go to 1300
 1100 ng=ng+1
! ng = number of the equivalent group of atoms respect all' operation IAX
! ng = numero del gruppo di atomi equivalente rispetto all'operazione IAX
      iv(i)=ng
      do j=1,3
         CO(j,ng)=XO(j,I)
      enddo
      W(ng)=PESO(I)
      nge(ng)=1
      k=I
 1200 l=MK(k,IAX)
      if(l.eq.I)go to 1000
      iv(l)=ng
! somma tutti gli atomi del gruppo con segno opportuno
      do j=1,3
         CO(j,ng)=CO(j,ng)+cost*XO(j,l)
      enddo
      W(ng)=W(ng)+PESO(l)
      nge(ng)=nge(ng)+1
      k=l
      go to 1200
! it has assigned to all atoms to a group and the weight to the groups
! ha assegnato tutti gli atomi ad un gruppo ed il peso ai gruppi 
 1300 continue    
      do i=1,ng
      W(I)=W(i)/nge(I)
      enddo
      call vrload(V3,0.d0,3)
      dism=0.d0
!it tries the group piu' far away dall' element of symmetry (im)
!erca il gruppo piu' lontano dall'elemento di simmetria (im)
      do i=1,ng 
         call prodmv(SIM,CO,VS,IAX,i,1)
         call LinearCombination(CO,VS,VS,1.d0,cost,i,1,1)
         call prods(VS,VS,dis,1,1,1)
         if(dis.gt.dism)then
            dism=dis
            im=i
         endif
      enddo
      call vrload(V3,0.d0,3)
         call prods(CO,CO,dim,im,im,2)
! somma fra loro tutti i gruppi con un segno determinato dall'angolo
! fra il gruppo in esame ed il gruppo im
      do i=1,ng
         call prods(CO,CO,abcos,im,i,1)
         call prods(CO,CO,di,i,i,2)
         absabcos=abs(abcos)
         if(absabcos.gt.1.d-5)then
            cost=W(I)*abcos/absabcos
            call LinearCombination(V3,CO,V3,1.d0,cost,1,i,1)
         endif
      enddo
! il vettore risultante e' normalizzato e diventera' una riga della
! matrice di rotazione definitiva
      call norm(V3,1)
      if(V3(1).gt.0.d0)return
      V3(1)=-V3(1)
      V3(2)=-V3(2)
      V3(3)=-V3(3)
      return
      end


!     ------------------------------------------------------------------
      SUBROUTINE prodmm(A,B,C,I,J,N)
!     ------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,3,*), B(3,3,*), C(3,3,*), X(3,3)

      DO  K = 1,3
         DO  L = 1,3
            X(K,L) = 0.0d0
            DO M = 1,3
               Y = A(K,M,I)
               Z = B(M,L,J)
               X(K,L) = X(K,L) + Y*Z
            ENDDO
         ENDDO
      ENDDO

      DO K = 1,3
         DO L = 1,3
            C(K,L,N) = X(K,L)
         ENDDO
      ENDDO

      RETURN
      END



!     ------------------------------------------------------------------
      SUBROUTINE prodmv(A,B,C,I,J,K)
!     ------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,3,*),B(3,*),C(3,*),X(3)

      DO L = 1,3
         X(L) = 0.0d0
         DO M = 1,3
            Y = A(L,M,I)
            Z = B(M,J)
            X(L) = X(L)+Y*Z
         ENDDO
      ENDDO

      DO L = 1,3
         C(L,K) = X(L)
      ENDDO

      RETURN
      END



!     ------------------------------------------------------------------
      subroutine prods(A,B,C,I,J,K)
!     ------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
!     PRODOTTO SCALARE A*B=C (K=1)
!     MODULO VETTORE A  (K=2)
      DIMENSION A(3,*), B(3,*)

      X = 0.0d0

      DO 1 L = 1,3
         Y = A(L,I)
         Z = B(L,J)
    1    X = X+Y*Z

      C = X

      IF (K.EQ.2) C = DSQRT(X)

      RETURN
      END


!     Vector cross product
      SUBROUTINE prodv(A,B,C,I,J,K)
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,1), B(3,1), C(3,1)

      X1 = A(1,I)
      X2 = A(2,I)
      X3 = A(3,I)
      Y1 = B(1,J)
      Y2 = B(2,J)
      Y3 = B(3,J)
      C(1,K) =  X2*Y3 - X3*Y2
      C(2,K) = -X1*Y3 + X3*Y1
      C(3,K) =  X1*Y2 - X2*Y1

      RETURN
      END


!     ------------------------------------------------------------------
!     Rotates the reference and coordinates
      SUBROUTINE RotateFrame(CO,XO,N)
      USE GlobalArrays
!     ------------------------------------------------------------------

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION CO(3,3), XO(3,*)

      DO I = 1,N
         CALL prodmv(CO,XO,XO,1,I,I)
      ENDDO

      CALL prodmm(CO,OR,OR,1,1,1)
      CALL prodmm(CO,OT,OT,1,1,1)
      CALL Invert3x3(OT,OTI,1,1)

      RETURN 
      END




!     ------------------------------------------------------------------
      SUBROUTINE rms_min(tm)
!     ------------------------------------------------------------------

      USE GlobalArrays

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION tm(3), t(3)

      passo = 0.02
      CALL vrload(RV,0.d0,9)
      CALL vrload(tm,0.d0,3)
      rm = crms(tm)

    1 ind1=0

      DO i1 = 1,3
         t(1) = tm(1)+(2-i1)*passo
         DO i2 = 1,3
            t(2)=tm(2)+(2-i2)*passo
            DO i3=1,3
               t(3)=tm(3)+(2-i3)*passo
               arms=crms(t)
               IF (arms.lt.rm) THEN
                  ind1=1
                  rm = arms
                  CALL VectorCopy(t,tm,1,1)
               ENDIF
            ENDDO
         ENDDO
      ENDDO

      IF (ind1.eq.1) GOTO 1
      IF (passo.lt.0.0002) RETURN
      passo = passo*0.5
      GOTO 1

      END



!     ------------------------------------------------------------------
      subroutine s_coor(SI,XO,XS,CO,D,DEL,DLM,CSMloc,MK,ID,ISU,nes,N)
!     ------------------------------------------------------------------
      USE GlobalArrays

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION SI(3,3,NMG), XO(3,NMA),  XS(3,NMA), CO(3,NMA), D(NMA)
      DIMENSION DEL(4), DLM(4), MK(NMA,NMG), ID(4), ISU(NMG)

      REAL*8 :: V3(3)
      REAL*8, DIMENSION (:), ALLOCATABLE :: dc
      REAL*8, DIMENSION (:), ALLOCATABLE :: cf

      ALLOCATE (cf(nma))
      ALLOCATE (dc(nma))

!     Allocate(?) the symmetrized coordinates
      NAZ = 3*N

      CALL vrload(CO,  0.d0, NAZ)
      CALL vrload(XS,  0.d0, NAZ)
      CALL vrload(RMS, 0.d0, NAZ)
      CALL vrload(DEL, 0.d0, 4  )
      CALL vrload(DLM, 0.d0, 4  )
      CALL vrload(cf,  0.d0, N  )

!     ISU=indice delle matrici del sottogruppo in considerazione
!     nes=numero di elementi del sottogruppo
      DO I = 1,N
         CALL prods(XO,XO,D(I),I,I,2)
         DO J = 1,nes
            K = MK(I,ISU(j))
            CALL prodmv(SI,XO,V3,ISU(J),I,1)
            CALL LinearCombination(XS,V3,XS,1.d0,1.d0,K,1,K)
         ENDDO
      ENDDO

      DO i = 1,N
         DO J = 1,3
            XS(J,I) = XS(J,I)/dfloat(nes)
         ENDDO
         CALL prods(XS,XS,dc(i),i,i,2)
      ENDDO

      DO I=1,N
         DO J=1,nes
            K=MK(I,ISU(j))
            cf(I)=cf(I)+D(K)
         ENDDO
         IF (dc(i).gt.1.d-01) cf(i) = cf(i)/(dc(i)*nes)
         DO J=1,3
            XS(J,I)=XS(J,I)*cf(i)
         ENDDO
      ENDDO

! il fattore di correzione cf=media delle distanze atomi equivalenti baricentro,
! corregge le coordinate simmetrizzate facendo si che queste equivalgano alla
! media per pura rotazione degli atomi equivalenti
!alcola le rms e gli scostamenti medi (DEL) e massimi DLM
      do j=1,4
       ID(j)=0
      enddo
      DO 2820 i=1,N
       do j=1,3
        V3(j)=XO(j,i)-XS(j,i)
        com=dabs(V3(j))
        DEL(j)=DEL(J)+com
        if(com.gt.DLM(j))then
          DLM(j)=com
          ID(j)=I
        endif
       enddo
! massima deviazione dalla simmetria in angstrom
      call prods(V3,V3,com,1,1,2)

      DEL(4)=DEL(4)+com

      IF (DLM(4).lt.com) THEN
         DLM(4) = com
         ID(4)=I
      ENDIF

 2820 CONTINUE

!alcola la continuous symmetry measure (CSM)
      CSMloc = 0.d0

      DO j = 1,nes
         DO i = 1,N
            K = MK(i,ISU(j))
            call prodmv(SI,XO,V3,ISU(J),i,1)
            call LinearCombination(XS,V3,V3,1.d0,-1.d0,K,1,1)
            RMS(1,K) = RMS(1,K)+V3(1)*V3(1)
            RMS(2,K) = RMS(2,K)+V3(2)*V3(2)
            RMS(3,K) = RMS(3,K)+V3(3)*V3(3)
            call prods(V3,V3,com,1,1,1)
            CSMloc = CSMloc+com
         ENDDO
      ENDDO

      cost = 1.d0/dfloat(nes)
      RMST = 0.d0

      DO I = 1,N
         DO j = 1,3
            RMST=RMST+RMS(j,i)
            RMS(j,i)=sqrt(RMS(j,i)*cost)
         ENDDO
      ENDDO

      RMST = sqrt(RMST/dfloat(nes*N))
      CSMloc  = CSMloc*1.d2/dfloat(nes*N)

      DEALLOCATE (dc, cf)

      RETURN
      END



!     Determines the Schoenflies symbol from the group matrices
!     ------------------------------------------------------------------
      SUBROUTINE schoenfl(MDEG, pointGroup)
!     ------------------------------------------------------------------

      USE GlobalArrays

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      CHARACTER*3 lbls
      CHARACTER*3 pointGroup
      CHARACTER S1, S2, Sn*2, tot*4
      CHARACTER*5 pointgr(32)
      DIMENSION  lbls(NMG)

      DATA (pointgr(i), i=1,32) &
       /'C1 ', 'Ci ', 'C2 ', 'Cs ', 'C2h', 'D2 ', 'C2v', 'D2h', 'C4 ', &
        'S4 ', 'C4h', 'D4 ', 'C4v', 'D2d', 'D4h', 'C3 ', 'C3i', 'D3 ', &
        'C3v', 'D3d', 'C6 ', 'C3h', 'C6h', 'D6 ', 'C6v', 'D3h', 'D6h', &
        'T  ', 'Th ', 'O  ', 'Td ', 'Oh '/

      maxasp  = 0
      maxasi  = 0
      mplane  = 0
      invers  = 0
      nplane  = 0
      lbls(1) = 'E'

!     linear groups for these groups not stamp the symmetry matrices but only the group
      IF (MDEG.eq.5) THEN
         pointGroup = 'Civ'
         DO i = 1,NMS
            CALL ax_order(SIM,i,m,msign,inv)
            IF (inv.eq.1) GOTO 1000
         ENDDO
         NMS=1
         GOTO 5200
 1000    pointGroup = 'Dih'
         NMS = 1
         GOTO 5200
      ENDIF

      IF (NMS.eq.1) THEN
         pointGroup = 'C1 '
         IPOGROU = 1
         RETURN
      ENDIF
 
      do i=2,NMS
        tot='    '
        S1=' '
        S2=' '
        Sn='  '
        call ax_order(SIM,i,m,msign,inv)
        if(m.gt.maxasp.and.msign.eq.1)maxasp=m
        if(m.gt.maxasi.and.msign.eq.-1)maxasi=m
        if(m.eq.2.and.mplane.eq.0)mplane=1
!     write(*,'(a20,4i3)')'i,msign,m,inv',i,msign,m,inv
        S1='C'
        if(m.eq.2)then
!entro
          if(inv.eq.1)then
            invers=1
            Sn='i '
            go to 1100
          endif
! piano
          Sn='2 '
! asse 2
          if(msign.eq.-1)then
            Sn='s '
            nplane=nplane+1
          endif
          go to 1100
        endif
! asse improprio m>2
        write(Sn,'(i2)') m
        if(msign.eq.-1)S1='S'
 1100   tot=S1//Sn//S2
        call compatta(tot,4,k)
        lbls(i)=tot(1:3)
      enddo
!     write(*,*)'maxasp maxasi mplane nplane invers   MDEG    NMS'
!     write(*,'(7i7)')maxasp,maxasi,mplane,nplane,invers,MDEG,NMS
      pointGroup = '   '

      IF (NMS.eq.48)   pointGroup = 'Oh '
      IF (NMS.eq.60)   pointGroup = 'I  '
      IF (NMS.eq.120)  pointGroup = 'Ih '
      IF (pointGroup .ne. '   ') GOTO 5100

      IF (MDEG.eq.3) THEN
         IF (NMS.eq.12)   pointGroup = 'T23'
         IF (invers.eq.1) pointGroup = 'Th '
         IF (maxasi.eq.4) pointGroup = 'Td '
         IF (pointGroup .eq. '   ') pointGroup = 'O  '
         GOTO 5100
      ENDIF

      IF (NMS.eq.2) THEN
        pointGroup = lbls(2)
        GOTO 5000
      ENDIF

      S2 = ' '
      S1 = 'C'

!     Cn e S2n dove n=maxasp
      write(Sn,'(I2)')maxasp
      if(MOD(NMS,2).eq.0.and.maxasi.eq.NMS)then
        S1='S'
        write(Sn,'(I2)')maxasi
        if(NMS.ne.6)go to 5000
        pointGroup = 'C3i'
        go to 5100
      endif
      if(NMS.eq.maxasp)go to 5000
      if(NMS.lt.maxasi)then
        write(Sn,'(I2)')maxasi
        S1='S'
        go to 5000
      endif
! Dn, Cnv e Cnh
      S1='D'
      if(NMS.eq.maxasp*2)then
        if(nplane.eq.0)go to 5000
        S1='C'
        S2='v'
        if(nplane.eq.maxasp)go to 5000
        S2='h'
        go to 5000
      endif
      S2='h'
      if(nplane.eq.maxasp)S2='d'
 5000 tot='    '
      tot=S1//Sn//S2
      call compatta(tot,4,k)
      pointGroup = tot(1:3)

 5100 CONTINUE

 5200 WRITE(*,1) pointGroup, CSMT, RMST

    1 FORMAT('Schoenflies symbol = ',a7,'  CSM =',f8.4,5x, & 
         'Molecular RMS =',f8.4)

      RETURN
      END




!     ------------------------------------------------------------------
      SUBROUTINE MatrixCopy(A,B,I,J)
!     ------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,3,*), B(3,3,*)

      DO K = 1,3
         DO L = 1,3
            B(K,L,J) = A(K,L,I)
         ENDDO
      ENDDO

      RETURN
      END


!     ------------------------------------------------------------------
      SUBROUTINE VectorCopy(A,B,I,J)
!     ------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,1), B(3,1)

      B(1,J) = A(1,I)
      B(2,J) = A(2,I)
      B(3,J) = A(3,I)

      RETURN
      END


!     ------------------------------------------------------------------
      SUBROUTINE TransposeMatrix(A,B,M,N)
!     ------------------------------------------------------------------
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION A(3,3,*), B(3,3,*), C(3,3)

      DO I = 1,3
         DO J = 1,3
            C(I,J) = A(J,I,M)
         ENDDO
      ENDDO

      CALL MatrixCopy(C,B,1,N)

      RETURN
      END


!     VERIFICA SE A E' UNA MATRICE DI SIMMETRIA A MENO DI DXM
!     IN CASO AFFERMATIVO PONE MV=1,  NMS=NMS + 1, SIM(I,J,NMS=A(I,J)
!     MK(I,NMS) CONTIENE IL NUMERO DELL'ATOMO OTTENUTO STASFORMANDO I CON
!     L'OPERAZIONE NMS
!     ------------------------------------------------------------------
      SUBROUTINE verify(XO,A,MK,MN,MV,N)
!     ------------------------------------------------------------------

      USE GlobalArrays

      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      DIMENSION XO(3,1), A(3,3), MK(NMA,1), MN(1)

      REAL*8 :: V(3), W(3)
      INTEGER, DIMENSION (:), ALLOCATABLE :: iat

      ALLOCATE (iat(nma))

      MV = 1
!     controlla per prima cosa che ls matrice A non sia gia' compresa in SIM
      DO i = 1,NMS
         IF (ium(A,SIM,1.d-2,1,i).eq.1) GOTO 6
      ENDDO

      NMS = NMS+1

      DO 4 I = 1,N
         MK(I,NMS) = 0
         JJ = 0
         FMIN = DXM(I)
         CALL prodmv(A,XO,V,1,I,1)

         DO 2 J=1,N
            IF (MN(I).NE.MN(J)) GOTO 2
            call LinearCombination(XO,V,W,1.d0,-1.d0,J,1,1)
            call prods(W,W,F,1,1,2)
            IF (F.GT.FMIN) GO TO 2
            FMIN = F
            JJ = J
    2    CONTINUE
         IF (JJ.EQ.0) GOTO 5
         MK(I,NMS) = JJ
    4 CONTINUE

!     Analysis of the multiplication matrix

      DO k = 1,N
         iat(k) = 0
      ENDDO

      DO 10 k = 1,N
         if (iat(k).ne.0) GOTO 10
         iat(k) = 1
         neq = 1
         l = k
    7    m = MK(l,NMS)
         if (m.eq.k) GOTO 9
         iat(m) = 1
         neq = neq+1
         if (neq.gt.N) GOTO 5
         l = m
         GOTO 7
    9 CONTINUE

      call ax_order(A,1,nord,msign,inv)
! il numero di atomi equivalenti per l'operazione di ordine m puo' essere
! solo 1 o 2 (atomi giacenti sull'elemento di simmetria) m (caso normale)
! o 2m ( quando si ha un elemento riducibile 3/m 5/m etc.)
      if(neq.eq.1.or.neq.eq.2)go to 10
      if(neq.eq.nord)go to 10
      if(neq.eq.2*nord)go to 10
      go to 5

   10 continue

      call MatrixCopy(A,SIM,1,NMS)

      DEALLOCATE (iat)
      RETURN

    5 NMS=NMS-1
    6 MV=0

      DEALLOCATE (iat)
      RETURN
      END


!     X       = fractional coordinates originate them
!     NA      = Total number of atoms
!     MSP(I)  = pointer of the species of atom I
!     AMAS(I) = mass of atom I
!     XO      = coordinate of the singel atoms to symmetrize
!     N       = number of atoms to symmetrize
!     SIM     = symmetry matrices of the molecular group
!     NMS     = Order of the symmetry group
!     ------------------------------------------------------------------
      SUBROUTINE work(Coordinates, PointGroup)
!     ------------------------------------------------------------------

      USE GlobalArrays
      IMPLICIT DOUBLE PRECISION (a-h,o-z)
      PARAMETER (maxorder=8)

      CHARACTER*3 lbls
      CHARACTER*3 pointGroup
      
      DIMENSION Coordinates(3,NAtoms)

      INTEGER, DIMENSION (:,:), ALLOCATABLE :: MK
      INTEGER, DIMENSION (:),   ALLOCATABLE :: MN
      INTEGER, DIMENSION (:),   ALLOCATABLE :: IEQAT
      INTEGER, DIMENSION (:,:), ALLOCATABLE :: MD
      INTEGER, DIMENSION (:),   ALLOCATABLE :: ISU
      INTEGER, DIMENSION (:),   ALLOCATABLE :: IASU

      REAL*8, DIMENSION (:,:), ALLOCATABLE :: XO
      REAL*8, DIMENSION (:,:), ALLOCATABLE :: XS
      REAL*8, DIMENSION (:),   ALLOCATABLE :: D
      REAL*8, DIMENSION (:,:), ALLOCATABLE :: CO
      REAL*8, DIMENSION (:,:), ALLOCATABLE :: delta
      REAL*8, DIMENSION (:),   ALLOCATABLE :: PESO

      DIMENSION V(3), W(3), DEL(4), AA(3,3), AL(3,3), DELM(4), IDM(4)
      CHARACTER type(2)


      ALLOCATE (MK(nma, nmg))
      ALLOCATE (MN(nma))
      ALLOCATE (IEQAT(NMA))
      ALLOCATE (MD(NMA,2))
      ALLOCATE (ISU(NMG))
      ALLOCATE (IASU(NMA))

      ALLOCATE (XO(3, NMA))
      ALLOCATE (XS(3, NMA))
      ALLOCATE (D(NMA))
      ALLOCATE (CO(3, NMA))
      ALLOCATE (delta(3, NMA))
      ALLOCATE (PESO(NMA))


      MO = 1

      type(1)=' '
      type(2)='*'
      PIG = 3.14159265358979D0
      deliner=0.1d0

      call vrload(SIM,0.d0,9*NMG)
      call vrload(AL,0.d0,9)

      SIM(1,1,1) = 1.d0
      SIM(2,2,1) = 1.d0
      SIM(3,3,1) = 1.d0
      AL(1,1) = 1.d0
      AL(2,2) = 1.d0
      AL(3,3) = 1.d0

      F   = 0.d0 
      IES = 0 
      N   = 0 
      NMS = 1

! selezione gruppo atomi da simmetrizzare
      DO 450 i = 1,NAtoms
         call vrload(AA,0.d0,9)
         AA(1,1) = SX(1,I)
         AA(2,2) = SX(2,I)
         AA(3,3) = SX(3,I)
         call prodmv(O,SX,AA,1,I,1)
         SIG(I) = 0.d0
         den = 0.d0

         do 435 k=1,3
            if(AA(K,K).le.0.d0)go to 435
            SIG(I)=SIG(I)+AA(k,k)
            den=den+1.d0
  435    continue

      IF (den .ne. 0.d0) THEN
        SIG(I) = SIG(I)/den
      ENDIF

      if(MO.ne.1) goto 450

      N = N+1
      IASU(i)=2
      IEQAT(N)=i

      XO(1,N) = X(1,i)
      XO(2,N) = X(2,i)
      XO(3,N) = X(3,i)

      MN(N)=MSP(i)
      IF (indwgh.eq.1) PESO(N) = AMAS(i)
      IF (indwgh.eq.2) PESO(N) = 1.0D0

      MK(N,1) = N

  450 continue

      porig = PESO(1)

      IF (N .le. 2) THEN
         WRITE(*,*)' GROUP WITH LESS THAN THREE ATOMS'
      DEALLOCATE (MK, MN, IEQAT, MD, ISU, IASU)
      DEALLOCATE (XO, XS, D, CO, delta, PESO)
         RETURN
      ENDIF
!
!     ORTOGONALIZZAZIONE DELLE COORDINATE DEGLI ATOMI DA SIMMETRIZZARE
!     CALCOLO DELLA DISTANZA MEDIA DAL BARICENTRO DEL GRUPPO
!
  460 call momin(XO,PESO,O,N)
!
!     DM=maximum length of vectors XO(I)
!
      DM = 0.d0
      DO I = 1,N
         CALL prods(XO,XO,D(I),I,I,2)
         IF (D(i) .gt. DM) THEN
            DM=D(i)
         ENDIF
      ENDDO
!

      DO I = 1,N
         DXM(I) = DCM
         IF (indtol .eq. 2) DXM(I) = DCM*D(I)/DM
         IF (indtol .eq. 3) DXM(I) = SIG(I)*DCM
      ENDDO

      MDEG=0
      delr=DABS(RIN(1)-RIN(2))/RIN(1)

      IF(delr.LE.deliner) THEN
        MDEG=1
      ENDIF
      delr=DABS(RIN(2)-RIN(3))/RIN(1)
      IF(delr.LE.deliner)then
        MDEG=MDEG+2
      endif
      NDEGA=MDEG+1
      nd=(MDEG+1)/2+1

!atg  write(*,*)'PRINCIPAL INERTIA MOMENTS and DEGENERATION DEGREE'
!atg  write(*,'(3f10.2,i10,/)')RIN,nd
!_______________________________________________________________________
! gruppi lineari
      if(MDEG.eq.1)then
        do i=1,N
          com=DSQRT(XO(1,i)*XO(1,i)+XO(2,i)*XO(2,i))
          if(com.gt.DXM(i))go to 990
        enddo
        call vrload(CO,0.d0,9)
        CO(1,1)=-1.d0
        CO(2,2)=-1.d0
        CO(3,3)= 1.d0
        call verify(XO,CO,MK,MN,MV,N)
        CO(3,3)=-1.d0
        call verify(XO,CO,MK,MN,MV,N)
        MDEG=5
        go to 2520
      endif
!_______________________________________________________________________
  990 GO TO (1020,1020,1000,2000),NDEGA
!
! ASSE UNICO=X  RUOTA LE COORDINATE IN MODO DA AVERE Z COME ASSE UNICO
! MODIFICA DI CONSEGUENZA ANCHE LA MATRICE DI ORIENTAZIONE
!
 1000 call vrload(CO,0.d0,9)
      MDEG=1
      CO(3,1)=1.d0 
      CO(1,3)=1.d0 
      CO(2,2)=-1.d0
      call RotateFrame(CO,XO,N)
      COM=RIN(3)
      RIN(3)=RIN(1) 
      RIN(1)=com
! ASSE UNICO Z
 1020 IU=3
      IB=2 
      IC=1
! RICERCA DELL'ASSE DI ORDINE MORD
 1100 MORD=maxorder+1
      IF(MDEG.EQ.0)MORD=3
      NASS=1
 1110 MORD=MORD-1
      IF(MORD.EQ.3.AND.MDEG.EQ.3)MORD=2
      IF(MORD.EQ.1)GO TO 1210
! GENERAZIONE DELLA MATRICE DI SIMMETRIA
 1120 COST=1.d0
      call vrload(CO,0.d0,9)
      ROT=2.d0*PIG/dfloat(MORD)
      CA=COS(ROT)
      CB=SIN(ROT)
 1140 CO(IB,IB)=CA*COST
      CO(IC,IC)=CO(IB,IB)
      CO(IB,IC)=CB*COST
      CO(IC,IB)=-CO(IB,IC)
      CO(IU,IU)=COST
      call verify(XO,CO,MK,MN,MV,N)

      IF(MV.EQ.1)GO TO 1150
      IF(COST.EQ.-1.d0)GO TO 1110
      COST=-1.d0
      mmez=MORD/2
      m2=2*mmez
      if(m2.ne.MORD)go to 1140
! se MORD=2*mmez ed mmez=dispari (assi -6=3/m -10=5/m)  si ha un
! elemento riducibile quindi e' inutile testarlo
      m2=2*mmez/2
      if(mmez-m2.eq.1)go to 1110
      GO TO 1140
! HA TROVATO UN ELEMENTO DI SIMMETRIA
 1150 IF(MORD.EQ.3.OR.MORD.EQ.6)IES=1

      if(MDEG.eq.1.and.MORD.gt.2)go to 1300
! RICERCA ELEMENTI DI SIMMETRIA SU NUOVI ASSI
 1205 if(MORD.gt.2)go to 1110
 1210 NASS=NASS+1
! AL COMPLETAMENTO DEL GRUPPO
      IF(NASS.EQ.4)GO TO 2500
! RICICLO SUL SECONDO O TERZO ASSE
      MORD=2
      I=IU 
      IU=IB 
      IB=IC 
      IC=I
      GO TO 1120

! RICERCA DELLA ROTAZIONE PER PORTARE EVENTUALI ELEMENTI DI SIMMETRIA
! A COINCIDERE CON GLI ASSI DI RIFERIMENTO
 1300 IF (N.le.1) THEN
         WRITE(*,*)'Single atom, not a molecule'
         DEALLOCATE (MK, MN, IEQAT, MD, ISU, IASU)
         DEALLOCATE (XO, XS, D, CO, delta, PESO)
         RETURN
      ENDIF

! SELEZIONE DELL'ATOMO DA USARE PER DETERMINARE LA ROTAZIONE
      call vrload(CO,0.d0,18)
      CO(3,3)=1.d0
! divisione in gruppi equidistanti dall'asse unico
      DO I = 1,N
        D(I) = DSQRT(XO(1,I)*XO(1,I)+XO(2,I)*XO(2,I))
        MD(I,1) = 0
        MD(I,2) = 0
      ENDDO
      IAI = 0
! IAI=NUMERO DI ENNUPLE DI ATOMI AVENTI DISTANZE COMPRESE IN UN RANGE
!         X< D <X+DXM*.5
! MD(I,1)=NUMERO DELLA ENNUPLA DI CUI L'ATOMO I FA PARTE
! MD(IAI,2)=N PER IL PRIMO ATOMO DELLA ENNUPLA DOVE N E' IL NUMERO DI
!         ATOMI DELLA STESSA
      N1=N-1
      do 1320 I=1,N1
      IF(MD(I,1).NE.0)GO TO 1320
      IAI=IAI+1
      MD(I,1)=IAI
      MD(IAI,2)=1
      J=I+1
      DO 1310 K=J,N
         IF(MN(I).NE.MN(K).or.K.eq.I.or.MD(K,1).ne.0)GO TO 1310
      IF(ABS(D(I)-D(K)).GT.0.5d0*DXM(I)) GO TO 1310
      MD(K,1)=IAI
      MD(IAI,2)=MD(IAI,2)+1
 1310 CONTINUE
 1320 CONTINUE
! ricerca del sottogruppo minimo
      JJ=10000
      do 1330 i=1,IAI
      if(MD(I,2).ge.JJ.or.MD(I,2).le.2)go to 1330
      JJ=MD(I,2)
 1330 continue
! fra i sottogruppi minimi viene scelto il piu' distante
! dall'asse unico
      dm=0.
      do 1350 I=1,IAI
      if(MD(I,2).ne.JJ)go to 1350
      do 1340 k=1,N
      if(MD(k,1).ne.I)go to 1340
        if(D(k).gt.dm)then
        dm=D(k)
        II=I
      endif
 1340 continue
 1350 continue

      call vrload(AA,0.d0,9)

      N1=N-1
      DO 1370 I1=1,N1
      IF(MD(I1,1).NE.II)GO TO 1370
      IN1=I1+1
      DO 1360 I2=IN1,N
      IF(MD(I2,1).NE.II)GO TO 1360
      call LinearCombination(XO,XO,CO,1.d0,1.d0,I1,I2,1)    
      call prodv(CO,CO,CO,3,1,2)
      cam=CO(1,2)*CO(1,2)+CO(2,2)*CO(2,2)+CO(3,2)*CO(3,2)
      if(cam.le.0.00001)go to 1360
      call norm(CO,2)
      call prodv(CO,CO,CO,2,3,1)
      call TransposeMatrix(CO,CO,1,1)
      call RotateFrame(CO,XO,N)
      com=1.d0
 1355 AA(1,1)=-1
      AA(2,2)=-1
      AA(3,3)= com
      call verify(XO,AA,MK,MN,MV,N)

      AA(1,1)=-1
      AA(2,2)= 1
      AA(3,3)= com
      call verify(XO,AA,MK,MN,MV,N)
      AA(1,1)= 1
      AA(2,2)=-1
      AA(3,3)= com
      call verify(XO,AA,MK,MN,MV,N)
      AA(1,1)= 0
      AA(1,2)= 1
      AA(2,1)= 1
      AA(2,2)= 0
      AA(3,3)= com
      call verify(XO,AA,MK,MN,MV,N)
      if(NMS.ge.3)go to 1110
      if(com.le.0.0)go to 1360
      com=-com
      call vrload(AA,0.d0,9)
      go to 1355
!di ognuna di queste quattro  matrici fare l'equivalente con -z!!!!!
 1360 continue
 1370 continue
      go to 2500
! 
!ASO TRIPLAMENTE DEGENERE
 2000 N1=N-1
      N2=N-2
      DO 2010 I=1,N
      MD(I,1)=0
      MD(I,2)=0
      DO 2010 J=2,NMG
 2010 MK(I,J)=0
      IAI=0
! IAI=NUMERO DI ENNUPLE DI ATOMI AVENTI DISTANZE COMPRESE IN UN RANGE
!         X< D <X+DXM*.5
! MD(I,1)=NUMERO DELLA ENNUPLA DI CUI L'ATOMO I FA PARTE
! MD(IAI,2)=N PER IL PRIMO ATOMO DELLA ENNUPLA DOVE N E' IL NUMERO DI
!         ATOMI DELLA STESSA
      DO 2050 I=1,N1
      IF(MD(I,1).NE.0)GO TO 2050
      IAI=IAI+1
      MD(I,1)=IAI
      MD(IAI,2)=1
      J=I+1
      DO 2040 K=J,N
      IF(MN(I).NE.MN(K).or.K.eq.I.or.MD(K,1).ne.0)GO TO 2040
      IF(ABS(D(I)-D(K)).GT.0.5d0*DXM(I)) GO TO 2040
      MD(K,1)=IAI
      MD(IAI,2)=MD(IAI,2)+1
 2040 CONTINUE
 2050 CONTINUE
! SE CI SONO ATOMI SU POSIZIONI SPECIALI USA QUESTI PER TROVARE LA
! MATRICE DI ROTAZIONE
 2100 call vrload(CO,0.d0,9)
      mmd=1000
      do i=1,IAI
        if(MD(I,2).lt.mmd.and.MD(I,2).gt.1)then
          mmd=MD(I,2)
          II=i
        endif
      enddo
! perche' ci sia simmetria I o Ih ci devono essere almeno 12 atomi equivalenti
      if(MD(II,2).eq.12.or.MD(II,2).eq.20.or.MD(II,2).eq.30.or.MD(II,2).gt.48) &
        call icosahed(XO,PESO,N,MN,MK,MD,II,MDEG,*2780)

      if(MDEG.eq.1) go to 1020

      if(NMS.ge.5)then
        MDEG=1
        call vrload(AA,0.d0,9)
        write(*,'(i3,3f9.5)')(I,(XO(k,I),k=1,3),i=1,N)
        AA(1,1)= 1
        AA(2,2)= 1
        AA(3,3)=-1
        call verify(XO,AA,MK,MN,MV,N)
        AA(1,1)= 1
        AA(2,2)=-1
        AA(3,3)= 1
        call verify(XO,AA,MK,MN,MV,N)
        AA(1,1)=-1
        AA(2,2)= 1
        AA(3,3)= 1
        call verify(XO,AA,MK,MN,MV,N)
        go to 2500
      endif
!
! RICERCA ASSE 3 SU ATOMI IN POSIZIONE GENERALE
! NOTA: TRE ATOMI EQUIVALANTI PER UN ASSE 3 DEVONO FORMARE UN TRIANGOLO
! EQUILATERO
! II E' IL NUMERO DI ATOMI DEL SOTTOGRUPPO
!
 2110 CA=0.d0
! matrice di rotazione 3 coincidente con z
      call vrload(AA,0.d0,9)
      AA(3,3)=1.d0
      AA(1,1)=-.5d0
      AA(2,2)=-.5d0
      AA(2,1)=0.5d0*DSQRT(3.d0)
      AA(1,2)=-AA(2,1)
      NMS1=1
!
!     I1,I2,I3=INDICATORI DEI TRE ATOMI POSSIBILE GENERATORI DELL'ASSE 3
!
      N2=N-2
      N1=N-1
      DO 2200 I1=1,N2
      IF(MD(I1,1).NE.II)GO TO 2200
      IN1=I1+1
      DO 2190 I2=IN1,N1
      IF(MD(I2,1).NE.II)GO TO 2190
      call LinearCombination(XO,XO,CO,1.d0,-1.d0,I1,I2,4)
      call prods(CO,CO,CA,4,4,2)
      IN2=I2+1
      DO 2180 I3=IN2,N
      IF(MD(I3,1).NE.II)GO TO 2180
      call LinearCombination(XO,XO,CO,1.d0,-1.d0,I1,I3,5)
      call prods(CO,CO,CB,5,5,2)
      call LinearCombination(XO,XO,CO,1.d0,-1.d0,I2,I3,6)
      call prods(CO,CO,CC,6,6,2)
      IF(DABS(CA-CB).GT.DXM(I1))GO TO 2180
      IF(DABS(CA-CC).GT.DXM(I1))GO TO 2180
      IF(DABS(CB-CC).GT.DXM(I1))GO TO 2180
      CO(1,3)=XO(1,I1)+XO(1,I2)+XO(1,I3)
      CO(2,3)=XO(2,I1)+XO(2,I2)+XO(2,I3)
      CO(3,3)=XO(3,I1)+XO(3,I2)+XO(3,I3)
      call norm(CO,3)
! salvataggio del possibile asse 3
      CO(1,6+NMS1)=CO(1,3)
      CO(2,6+NMS1)=CO(2,3)
      CO(3,6+NMS1)=CO(3,3)
!ontrollo che l'asse trovato non coincida gia' con z cioe'
!on il primo asse C3 trovato
      if(NMS1.gt.1)then
        call prods(CO,CO,pro,7,8,1)
        if(DABS(pro).gt.0.9d0)go to 2180
        if(pro.gt.0.d0)then
          CO(1,8)=-CO(1,8)
          CO(2,8)=-CO(2,8)
          CO(3,8)=-CO(3,8)
        endif
      endif
! definizione di un secondo asse
      do i=1,3
        call vrload(CO,0.d0,3)
        CO(I,1)=1.d0
        call prods(CO,CO,pro,1,3,1)
        if(DABS(pro).gt.0.5)go to 2150
      enddo
! verifica se l' asse trovato e' un C3
! rotazione per portare il C3 a coincidere con z
 2150 call prodv(CO,CO,CO,3,1,2)
      call norm(CO,2)
      call prodv(CO,CO,CO,2,3,1)
      call TransposeMatrix(CO,CO,1,1)
      call RotateFrame(CO,XO,N)
      call verify(XO,AA,MK,MN,MV,N)
      NMS=1
      NMS1=NMS1+MV
      call TransposeMatrix(CO,CO,1,1)
      call RotateFrame(CO,XO,N)
      if(NMS1.eq.3)GO TO 2350
 2180 CONTINUE
 2190 CONTINUE
 2200 CONTINUE
      write(*,2)
    2 format('********************** WARNING **********************',//, &
      '       INCREASING THE TOLERANCE COULD BE USEFUL',//, &
      '*****************************************************')
      if(NMS1.eq.1)then
! se ha gia' modificato una volta i pesi non li modifica ulteriormente
! ma riduce la tolleranza accettata fra i momenti di inerzia per il
!alcolo della degenerazione
        if(porig.ne.PESO(1))then
          deliner=deliner*0.1d0
          go to 460
        endif
! 
! pseudodegenerazione 3 senza assi C3. Esiste ancora la possibilita'
! che la pseudodegenerazione sia completa (MDEG=0) o che ci sia una
! asse 4,-4,5,-5,7,-7,8,-8
! 
        write(*,*)'Weights are changed'
        do k=1,N
          PESO(K)=PESO(K)*(D(k)/DM)**4
        enddo
        go to 460
      endif
! ha trovato un solo asse 3 [ = CO(7)] e si riporta in quel
! riferimento ripristinando NMS
! definizione di un secondo asse
      CO(1,3)=CO(1,7)
      CO(2,3)=CO(2,7)
      CO(3,3)=CO(3,7)
      do i=1,3
        call vrload(CO,0.d0,3)
        CO(I,1)=1.d0
        call prods(CO,CO,pro,1,3,1)
        if(DABS(pro).gt.0.5)go to 2310
      enddo
 2310 call prodv(CO,CO,CO,3,1,2)
      call norm(CO,2)
      call prodv(CO,CO,CO,2,3,1)
      call TransposeMatrix(CO,CO,1,1)
      call RotateFrame(CO,XO,N)
      go to 2490
! ha trovato due assi C3 [CO(7) e CO(8)] e li usa per
! determinare il riferimento definitivo
 2350 call LinearCombination(CO,CO,CO,1.d0,1.d0,7,8,1)
      call norm(CO,1)
      call LinearCombination(CO,CO,CO,1.d0,-1.d0,7,8,3)
      call prodv(CO,CO,CO,3,1,2)
      call norm(CO,2)
      call prodv(CO,CO,CO,1,2,3)
      call TransposeMatrix(CO,CO,1,1)
      call RotateFrame(CO,XO,N)
      call vrload(CO,0.d0,9)
      CO(1,1)=1.d0
      CO(2,2)=DSQRT(0.5d0)
      CO(3,3)=CO(2,2)
      CO(2,3)=CO(2,2)
      CO(3,2)=-CO(2,2)
      go to 2410
! ORTONORMALIZZAZIONE DELLA MATRICE DI ROTAZIONE
 2400 call norm(CO,5)
!     write(out,'(30i4)')(MD(i,2),i=1,N)
      call prodv(CO,CO,CO,5,6,7)
      call norm(CO,7)
      call prodv(CO,CO,CO,7,5,6)
      call VectorCopy(CO,CO,5,1)
      call VectorCopy(CO,CO,6,2)
      call VectorCopy(CO,CO,7,3)
      call TransposeMatrix(CO,CO,1,1)
! ROTAZIONE SU NUOVA TERNA
!     write(out,*)' CO prima di RotateFrame'
!     write(out,'(3f10.5)')((CO(kk,ii),ii=1,3),kk=1,3)
 2410 call RotateFrame(CO,XO,N)
! VERIFICA GENERALE ESISTENZA ASSE TERNARIO
!     write(out,*)' 2410 coordinate'
!     write(out,'(i2,3f9.5)')(lll,(XO(kkk,lll),kkk=1,3),lll=1,N)
      call vrload(AA,0.d0,9)
      AA(1,2)=1.d0
      AA(2,3)=1.d0
      AA(3,1)=1.d0
      call verify(XO,AA,MK,MN,MV,N)
!     write(out,*)' 2400 tentativo con la matrice:'
!     write(out,'(3f9.5)')((CO(kkk,lll),lll=1,3),kkk=1,3)
!     write(out,*)'NMS,MV',NMS,MV
      NASS=1
! ha torvato un asse 3 (diagonale) ora cerca un asse 2
      IU=1
      IB=2
      IC=3
      MORD=4
      call vrload(CO,0.d0,9)
      CO(1,1)=-1.d0
      CO(2,2)=-1.d0
      CO(3,3)=1.d0
      call verify(XO,CO,MK,MN,MV,N)
!     write(out,*)' 2400 tentativo con la matrice:'
!     write(out,'(3f9.5)')((CO(kkk,lll),lll=1,3),kkk=1,3)
!     write(out,*)'NMS,MV',NMS,MV
      if(MV.eq.1)GO TO 1120
! non esiste l'asse 2 allineato su x: si tratta di una pseudodegenerazione
! mi metto in modo che l'asse 3 sia allineato a z
      call vrload(CO,0.d0,9)
      call vrload(AA,0.d0,9)
      cost=dsqrt(0.5d0)
      CO(1,1)=cost
      CO(2,2)=cost
      CO(1,2)=-cost
      CO(2,1)= cost
      CO(3,3)= 1.d0
      ang=0.5d0*DACOS(-1.d0/3.d0)
      aaa=ang*180.d0/PIG
      cosa=DCOS(ang)
      sina=DSIN(ang)
      AA(2,2)=cosa
      AA(3,3)=cosa
      AA(2,3)=-sina
      AA(3,2)=sina
      AA(1,1)=1.d0
      call prodmm(AA,CO,CO,1,1,1)
      call RotateFrame(CO,XO,N)
 2490 NMS=1
      MORD=6
      MDEG=1
      IU=3
      IB=2
      IC=1
      write(*,5)
    5 format( &
      '***********************************************************',/, &
      'WARNING: the degeneration degree is 3 but no cubic',/,  &
      '         or icosahedral group can be found.',/,  &
      'IF YOU SUSPECT THE EXISTENCE OF ONE OF THEM, PLEASE CHANGE:',/,  &
      '1) the weighting scheme   OR',/,  &
      '2) put MOL<0 to the atoms farest from the baricenter   OR',/,  &
      '3) enlarge DCM',/,  &
      '***********************************************************') 
      go to 1120
! RICERCA DEL CENTRO DI SIMMETRIA
 2500 II=2
      call vrload(CO,0.d0,9)
      CO(1,1)=-1.d0
      CO(2,2)=-1.d0
      CO(3,3)=-1.d0
      call verify(XO,CO,MK,MN,MV,N)
      IF (NMS.NE.1) GOTO 2520

      WRITE(*,3)
      DEALLOCATE (MK, MN, IEQAT, MD, ISU, IASU)
      DEALLOCATE (XO, XS, D, CO, delta, PESO)
      RETURN 

!     complete the group
 2520 call CompleteGroup(MK,N,*4100)

!     Linear group
      IF (MDEG.eq.5) GOTO 2790
      ntest=0

      DO i = 1,NMS
        IF (MTG(i,i).ne.1) ntest=1
      ENDDO

! variazione per far si che in un C2 o C2v l'asse 2 coincida con z
      if(NMS.eq.2.or.NMS.eq.4)then
        nax2=0
        lax2=0
        do i=2,NMS
          itrac=NINT(SIM(1,1,i)+SIM(2,2,i)+SIM(3,3,i))
          iabst=NINT(DABS(SIM(1,1,i))+DABS(SIM(2,2,i))+DABS(SIM(3,3,i)))
          if(iabst.ne.3)go to 2740
          if(itrac.eq.-1)then
            nax2=nax2+1
            lax2=i
          endif
        enddo
        if(nax2.eq.0.or.nax2.eq.3)go to 2780
        if(nax2.eq.1)then
          do l=1,3
            if(NINT(SIM(l,l,lax2)).eq.1)go to 2730
          enddo
 2730     if(l.eq.3)go to 2780
          call vrload(CO,0.d0,9)
          i=3-l
          k=6-i-l
          CO(k,l)=1.d0
          CO(l,k)=-1.d0
          CO(i,i)=1.d0
          call RotateFrame(CO,XO,N)
          com=RIN(3)
          RIN(3)=RIN(l)
          RIN(l)=com
        endif
        NMS=1
        go to 1020
      endif
 2740 if(ntest.eq.0)go to 2780
!+++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++
! Ottimizzazione del riferimento per MDEG>0
 2780 if(MDEG.eq.0)go to 2790
      do i=1,NMS
       ISU(i)=i
      enddo
      call s_coor(SIM,XO,XS,CO,D,DEL,DELM,CSMT,MK,IDM,ISU,NMS,N)
      npb=0
      do k=1,NMS
       do i=1,N
        npu=npb+i
        call VectorCopy(XO,ppu,i,npu)
        npu=npb+MK(i,k)
        call prodmv(SIM,XS,ppo,k,i,npu)
       enddo
       npb=npb+N
      enddo
      npu=npb
      call rms_min(V)
      arms=crms(V)
!     write(6,*)'v,RV'
!     write(6,'(3f10.5)')V,RV
      call RotateFrame(RV,XO,N)
!alcolo simmetria per sottogruppi di operazioni
 2790 CSM(1)=0.d0
      DEV(1)=0.d0
!++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
!     write(out,*)'GROUP MULTIPLICATION TABLE'
!     write(out,*)
!     nst=NMS
!     nis=25
!     if(NMS.gt.24)nst=24
!     do i=1,NMS
!      write(out,'(24i3)')(MTG(i,j),j=1,nst)
!     enddo
!     write(out,*)
!     if(nst.ne.NMS)then
!      do i=1,NMS
!       write(out,'(24i3)')(MTG(i,j),j=nis,NMS)
!      enddo
!     endif
!++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++++ 
! simmetrizzazione per sottogruppi di una solo elemento (e sue potenze)

      DO 2950 I=2,NMS
         nes=1
         ISU(1)=nes
         k=i
 2800    nes=nes+1
         ISU(nes)=k
         l=MTG(i,k)

         IF (l.eq.1) go to 2900

         k=l
         go to 2800

 2900    call s_coor(SIM,XO,XS,CO,D,DEL,DELM,comt,MK,IDM,ISU,nes,N)

         DEV(I)=DELM(4)
         CSM(I)=comt
 2950 CONTINUE

!ALCOLA LE COORDINATEE SIMMETRIZZATE PER L'INTERO GRUPPO

 3000 DO i = 1,NMS
         ISU(i) = i
      ENDDO

      CALL s_coor(SIM,XO,XS,CO,D,DEL,DELM,CSMT,MK,IDM,ISU,NMS,N)

! Recupero degli atomi con MOL<0

      N1 = N


      DO i = 1,NAtoms
!     DO i = 1,NA
         IF (1.eq.-MO) THEN 
            N1 = N1+1
            IEQAT(N1) = i
            MN(N1)    = MSP(i)
            MK(N1,1)  = -N1
            CALL LinearCombination(X, BARC, XO, 1.d0, -1.d0, i, 1, N1)
            CALL prodmv(OT, XO, XO, 1, N1, N1)
            XS(1,N1) = XO(1,N1)
            XS(2,N1) = XO(2,N1)
            XS(3,N1) = XO(3,N1)
         ENDIF
      ENDDO

      NSTART = N+1

      IF (N1.eq.N) GOTO 3400

      DO I =NSTART,N1
         DO  L=2,NMS
            MK(i,l)=0
            FMIN=10000.
            call prodmv(SIM,XO,V,L,I,1)
            DO 3150 K = NSTART,N1
               IF(MN(I).NE.MN(K)) GO TO 3150
               call LinearCombination(XO,V,W,1.d0,-1.d0,K,1,1)
               call prods(W,W,F,1,1,2)
               if(F.le.FMIN)then
                 KK=K
                 FMIN=F
               endif
 3150       continue
            IF (FMIN .LT. 2.*DCME) THEN
               MK(I,L) = KK
            ELSE
               MK(I,1) = 0
            ENDIF
         ENDDO
      ENDDO


      DO 3350 i=NSTART,N1
         IF (MK(I,1).eq.0) GOTO 3350
         DO k=2,NMS
            CALL prodmv(SIM,XO,V,k,i,1)
            l = MK(i,k)
            CALL LinearCombination(XS,V,XS,1.d0,1.d0,l,1,l)
         ENDDO
 3350 CONTINUE

      do 3360 I = NSTART,N1
         if(MK(I,1).eq.0) go to 3360
         do k = 1,3
            XS(k,I)=XS(k,I)/dfloat(NMS)
         enddo
 3360 continue


 3400 continue

      CALL asymunit(MK,IASU,N1)

      DO i = 1,natoms
         call LinearCombination(XS,XO,delta,1.d0,-1.d0,i,i,i)
      ENDDO

      CALL schoenfl(MDEG, pointgroup)

      DO I = 1,natoms
         Coordinates(1,i) = XS(1,i)
         Coordinates(2,i) = XS(2,i)
         Coordinates(3,i) = XS(3,i)
      ENDDO

 4100 DEALLOCATE (MK, MN, IEQAT, MD, ISU, IASU)
      DEALLOCATE (XO, XS, D, CO, delta, PESO)

      RETURN 


    3 FORMAT (' NO SYMMETRY EXISTS WITHIN GIVEN TOLERANCE')
    6 FORMAT (/,' SYMMETRY GROUP MATRICES',/)
    7 FORMAT (3(1X,3F16.10,/))

      END
