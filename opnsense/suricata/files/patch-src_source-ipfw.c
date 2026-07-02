--- src/source-ipfw.c.orig	2026-05-19 06:59:36 UTC
+++ src/source-ipfw.c
@@ -263,6 +263,10 @@ TmEcode ReceiveIPFWLoop(ThreadVars *tv, void *data, vo
                 continue;
         }
 
+        if (TmThreadsCheckFlag(tv, THV_KILL) || TmThreadsCheckFlag(tv, THV_PAUSE)) {
+            SCReturnInt(TM_ECODE_OK);
+        }
+
         if ((pktlen = recvfrom(nq->fd, pkt, sizeof(pkt), 0,
                                (struct sockaddr *)&nq->ipfw_sin,
                                &nq->ipfw_sinlen)) == -1) {
@@ -360,6 +364,14 @@ TmEcode ReceiveIPFWThreadInit(ThreadVars *tv, const vo
         SCReturnInt(TM_ECODE_FAILED);
     }
 
+#ifdef PF_DIVERT
+    int one = 1;
+    if (setsockopt(nq->fd, SOL_SOCKET, SO_REUSEPORT_LB, &one, sizeof(one)) == -1) {
+        SCLogError("Can't set IPFW divert socket SO_REUSEPORT_LB: %s", strerror(errno));
+        SCReturnInt(TM_ECODE_FAILED);
+    }
+#endif
+
     nq->ipfw_sinlen=sizeof(nq->ipfw_sin);
     memset(&nq->ipfw_sin, 0, nq->ipfw_sinlen);
     nq->ipfw_sin.sin_family = PF_INET;
@@ -570,6 +582,12 @@ TmEcode IPFWSetVerdict(ThreadVars *tv, IPFWThreadVars 
                     SCReturnInt(TM_ECODE_FAILED);
                 case EHOSTDOWN:
                 case ENETDOWN:
+#ifdef PF_DIVERT
+                case EACCES:
+                case EHOSTUNREACH:
+                case ENETUNREACH:
+                case ENOBUFS:
+#endif
                     break;
             }
         }
