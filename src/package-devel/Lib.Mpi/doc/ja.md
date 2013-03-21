Lib.Mpi-mpich�̃o�C���f�B���O
====================
# MPI�N���X
## ���\�b�h

###@Public @Static float MPI.getWtime()

---

# MPIComm�N���X
## ���\�b�h

###@Public int MPIComm.barrier()

---

###@Public boolean MPIComm.send(MPIData buf, int count, int dest, int tag)

---

###@Public boolean MPIComm.recv(MPIData buf, int count, int src, int tag)

---

###@Public MPIRequest MPIComm.iSend(MPIData buf, int count, int dest, int tag)

---

###@Public MPIRequest MPIComm.iRecv(MPIData buf, int count, int src, int tag)

---

###@Public boolean MPIComm.bcast(MPIData sdata, int count, int root)

---

###@Public int MPIComm.getSize()

---

###@Public @Static MPIComm MPIComm.getWorld()

---

###@Public int MPIComm.getRank()

---

###@Public boolean MPIComm.scatter(MPIData sdata, int scount, MPIData rdata, int rcount, int root)

---

###@Public boolean MPIComm.gather(MPIData sdata, int scount, MPIData rdata, int rcount, int root)

---

###@Public boolean MPIComm.allGather(MPIData sdata, int scount, MPIData rdata, int rcount)

---

###@Public boolean MPIComm.allToAll(MPIData sdata, int scount, MPIData rdata, int rcount)

---

###@Public boolean MPIComm.reduce(MPIData sdata, MPIData rdata, int rcount, int op, int root)

---

###@Public boolean MPIComm.allReduce(MPIData sdata, MPIData rdata, int rcount, int op)

---

###@Public boolean MPIComm.scan(MPIData sdata, MPIData rdata, int rcount, int op)

---

# MPIRequest�N���X
## ���\�b�h

@Public boolean MPIRequest.wait()

---

@Public boolean MPIRequest.test()

---

@Public boolean MPIRequest.cancel()

---

# MPIData�N���X
## ���\�b�h

@Public @Static MPIData MPIData.fromBytes(Bytes b)

---

@Public @Static MPIData MPIData.newFloatArray(int n)

---

@Public @Static MPIData MPIData.newIntArray(int n)

---

@Public Bytes MPIData.toBytes()

---

@Public void MPIData.setf(int n, float v)

---

@Public void MPIData.seti(int n, int v)

---

@Public void MPIData.setOffset(int offset)

---

@Public float MPIData.getf(int n)

---

@Public int MPIData.geti(int n)

---

@Public int MPIData.getOffset()

---

@Public int MPIData.getSize()

---

