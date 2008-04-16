/* OCaml bindings for libvirt.
 * (C) Copyright 2007 Richard W.M. Jones, Red Hat Inc.
 * http://libvirt.org/
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA 02110-1301  USA
 */

/* Please read libvirt/README file. */

/*----------------------------------------------------------------------*/

CAMLprim value
ocaml_libvirt_get_version (value driverv, value unit)
{
  CAMLparam2 (driverv, unit);
  CAMLlocal1 (rv);
  const char *driver = Optstring_val (driverv);
  unsigned long libVer, typeVer = 0, *typeVer_ptr;
  int r;

  typeVer_ptr = driver ? &typeVer : NULL;
  NONBLOCKING (r = virGetVersion (&libVer, driver, typeVer_ptr));
  CHECK_ERROR (r == -1, NULL, "virGetVersion");

  rv = caml_alloc_tuple (2);
  Store_field (rv, 0, Val_int (libVer));
  Store_field (rv, 1, Val_int (typeVer));
  CAMLreturn (rv);
}

/*----------------------------------------------------------------------*/

/* Connection object. */

CAMLprim value
ocaml_libvirt_connect_open (value namev, value unit)
{
  CAMLparam2 (namev, unit);
  CAMLlocal1 (rv);
  const char *name = Optstring_val (namev);
  virConnectPtr conn;

  NONBLOCKING (conn = virConnectOpen (name));
  CHECK_ERROR (!conn, NULL, "virConnectOpen");

  rv = Val_connect (conn);

  CAMLreturn (rv);
}

CAMLprim value
ocaml_libvirt_connect_open_readonly (value namev, value unit)
{
  CAMLparam2 (namev, unit);
  CAMLlocal1 (rv);
  const char *name = Optstring_val (namev);
  virConnectPtr conn;

  NONBLOCKING (conn = virConnectOpenReadOnly (name));
  CHECK_ERROR (!conn, NULL, "virConnectOpen");

  rv = Val_connect (conn);

  CAMLreturn (rv);
}

CAMLprim value
ocaml_libvirt_connect_get_version (value connv)
{
  CAMLparam1 (connv);
  virConnectPtr conn = Connect_val (connv);
  unsigned long hvVer;
  int r;

  NONBLOCKING (r = virConnectGetVersion (conn, &hvVer));
  CHECK_ERROR (r == -1, conn, "virConnectGetVersion");

  CAMLreturn (Val_int (hvVer));
}

CAMLprim value
ocaml_libvirt_connect_get_max_vcpus (value connv, value typev)
{
  CAMLparam2 (connv, typev);
  virConnectPtr conn = Connect_val (connv);
  const char *type = Optstring_val (typev);
  int r;

  NONBLOCKING (r = virConnectGetMaxVcpus (conn, type));
  CHECK_ERROR (r == -1, conn, "virConnectGetMaxVcpus");

  CAMLreturn (Val_int (r));
}

CAMLprim value
ocaml_libvirt_connect_get_node_info (value connv)
{
  CAMLparam1 (connv);
  CAMLlocal2 (rv, v);
  virConnectPtr conn = Connect_val (connv);
  virNodeInfo info;
  int r;

  NONBLOCKING (r = virNodeGetInfo (conn, &info));
  CHECK_ERROR (r == -1, conn, "virNodeGetInfo");

  rv = caml_alloc (8, 0);
  v = caml_copy_string (info.model); Store_field (rv, 0, v);
  v = caml_copy_int64 (info.memory); Store_field (rv, 1, v);
  Store_field (rv, 2, Val_int (info.cpus));
  Store_field (rv, 3, Val_int (info.mhz));
  Store_field (rv, 4, Val_int (info.nodes));
  Store_field (rv, 5, Val_int (info.sockets));
  Store_field (rv, 6, Val_int (info.cores));
  Store_field (rv, 7, Val_int (info.threads));

  CAMLreturn (rv);
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRNODEGETFREEMEMORY
extern unsigned long long virNodeGetFreeMemory (virConnectPtr conn)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_connect_node_get_free_memory (value connv)
{
#ifdef HAVE_VIRNODEGETFREEMEMORY
  CAMLparam1 (connv);
  CAMLlocal1 (rv);
  virConnectPtr conn = Connect_val (connv);
  unsigned long long r;

  WEAK_SYMBOL_CHECK (virNodeGetFreeMemory);
  NONBLOCKING (r = virNodeGetFreeMemory (conn));
  CHECK_ERROR (r == 0, conn, "virNodeGetFreeMemory");

  rv = caml_copy_int64 ((int64) r);
  CAMLreturn (rv);
#else
  not_supported ("virNodeGetFreeMemory");
#endif
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRNODEGETCELLSFREEMEMORY
extern int virNodeGetCellsFreeMemory (virConnectPtr conn,
				      unsigned long long *freeMems,
				      int startCell, int maxCells)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_connect_node_get_cells_free_memory (value connv,
						  value startv, value maxv)
{
#ifdef HAVE_VIRNODEGETCELLSFREEMEMORY
  CAMLparam3 (connv, startv, maxv);
  CAMLlocal2 (rv, iv);
  virConnectPtr conn = Connect_val (connv);
  int start = Int_val (startv);
  int max = Int_val (maxv);
  int r, i;
  unsigned long long freemems[max];

  WEAK_SYMBOL_CHECK (virNodeGetCellsFreeMemory);
  NONBLOCKING (r = virNodeGetCellsFreeMemory (conn, freemems, start, max));
  CHECK_ERROR (r == -1, conn, "virNodeGetCellsFreeMemory");

  rv = caml_alloc (r, 0);
  for (i = 0; i < r; ++i) {
    iv = caml_copy_int64 ((int64) freemems[i]);
    Store_field (rv, i, iv);
  }

  CAMLreturn (rv);
#else
  not_supported ("virNodeGetCellsFreeMemory");
#endif
}

CAMLprim value
ocaml_libvirt_domain_get_id (value domv)
{
  CAMLparam1 (domv);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  unsigned int r;

  NONBLOCKING (r = virDomainGetID (dom));
  /* There's a bug in libvirt which means that if you try to get
   * the ID of a defined-but-not-running domain, it returns -1,
   * and there's no way to distinguish that from an error.
   */
  CHECK_ERROR (r == (unsigned int) -1, conn, "virDomainGetID");

  CAMLreturn (Val_int ((int) r));
}

CAMLprim value
ocaml_libvirt_domain_get_max_memory (value domv)
{
  CAMLparam1 (domv);
  CAMLlocal1 (rv);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  unsigned long r;

  NONBLOCKING (r = virDomainGetMaxMemory (dom));
  CHECK_ERROR (r == 0 /* [sic] */, conn, "virDomainGetMaxMemory");

  rv = caml_copy_int64 (r);
  CAMLreturn (rv);
}

CAMLprim value
ocaml_libvirt_domain_set_max_memory (value domv, value memv)
{
  CAMLparam2 (domv, memv);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  unsigned long mem = Int64_val (memv);
  int r;

  NONBLOCKING (r = virDomainSetMaxMemory (dom, mem));
  CHECK_ERROR (r == -1, conn, "virDomainSetMaxMemory");

  CAMLreturn (Val_unit);
}

CAMLprim value
ocaml_libvirt_domain_set_memory (value domv, value memv)
{
  CAMLparam2 (domv, memv);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  unsigned long mem = Int64_val (memv);
  int r;

  NONBLOCKING (r = virDomainSetMemory (dom, mem));
  CHECK_ERROR (r == -1, conn, "virDomainSetMemory");

  CAMLreturn (Val_unit);
}

CAMLprim value
ocaml_libvirt_domain_get_info (value domv)
{
  CAMLparam1 (domv);
  CAMLlocal2 (rv, v);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  virDomainInfo info;
  int r;

  NONBLOCKING (r = virDomainGetInfo (dom, &info));
  CHECK_ERROR (r == -1, conn, "virDomainGetInfo");

  rv = caml_alloc (5, 0);
  Store_field (rv, 0, Val_int (info.state)); // These flags are compatible.
  v = caml_copy_int64 (info.maxMem); Store_field (rv, 1, v);
  v = caml_copy_int64 (info.memory); Store_field (rv, 2, v);
  Store_field (rv, 3, Val_int (info.nrVirtCpu));
  v = caml_copy_int64 (info.cpuTime); Store_field (rv, 4, v);

  CAMLreturn (rv);
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRDOMAINGETSCHEDULERTYPE
extern char *virDomainGetSchedulerType(virDomainPtr domain,
				       int *nparams)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_domain_get_scheduler_type (value domv)
{
#ifdef HAVE_VIRDOMAINGETSCHEDULERTYPE
  CAMLparam1 (domv);
  CAMLlocal2 (rv, strv);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  char *r;
  int nparams;

  WEAK_SYMBOL_CHECK (virDomainGetSchedulerType);
  NONBLOCKING (r = virDomainGetSchedulerType (dom, &nparams));
  CHECK_ERROR (!r, conn, "virDomainGetSchedulerType");

  rv = caml_alloc_tuple (2);
  strv = caml_copy_string (r); Store_field (rv, 0, strv);
  free (r);
  Store_field (rv, 1, nparams);
  CAMLreturn (rv);
#else
  not_supported ("virDomainGetSchedulerType");
#endif
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRDOMAINGETSCHEDULERPARAMETERS
extern int virDomainGetSchedulerParameters (virDomainPtr domain,
					    virSchedParameterPtr params,
					    int *nparams)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_domain_get_scheduler_parameters (value domv, value nparamsv)
{
#ifdef HAVE_VIRDOMAINGETSCHEDULERPARAMETERS
  CAMLparam2 (domv, nparamsv);
  CAMLlocal4 (rv, v, v2, v3);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  int nparams = Int_val (nparamsv);
  virSchedParameter params[nparams];
  int r, i;

  WEAK_SYMBOL_CHECK (virDomainGetSchedulerParameters);
  NONBLOCKING (r = virDomainGetSchedulerParameters (dom, params, &nparams));
  CHECK_ERROR (r == -1, conn, "virDomainGetSchedulerParameters");

  rv = caml_alloc (nparams, 0);
  for (i = 0; i < nparams; ++i) {
    v = caml_alloc_tuple (2); Store_field (rv, i, v);
    v2 = caml_copy_string (params[i].field); Store_field (v, 0, v2);
    switch (params[i].type) {
    case VIR_DOMAIN_SCHED_FIELD_INT:
      v2 = caml_alloc (1, 0);
      v3 = caml_copy_int32 (params[i].value.i); Store_field (v2, 0, v3);
      break;
    case VIR_DOMAIN_SCHED_FIELD_UINT:
      v2 = caml_alloc (1, 1);
      v3 = caml_copy_int32 (params[i].value.ui); Store_field (v2, 0, v3);
      break;
    case VIR_DOMAIN_SCHED_FIELD_LLONG:
      v2 = caml_alloc (1, 2);
      v3 = caml_copy_int64 (params[i].value.l); Store_field (v2, 0, v3);
      break;
    case VIR_DOMAIN_SCHED_FIELD_ULLONG:
      v2 = caml_alloc (1, 3);
      v3 = caml_copy_int64 (params[i].value.ul); Store_field (v2, 0, v3);
      break;
    case VIR_DOMAIN_SCHED_FIELD_DOUBLE:
      v2 = caml_alloc (1, 4);
      v3 = caml_copy_double (params[i].value.d); Store_field (v2, 0, v3);
      break;
    case VIR_DOMAIN_SCHED_FIELD_BOOLEAN:
      v2 = caml_alloc (1, 5);
      Store_field (v2, 0, Val_int (params[i].value.b));
      break;
    default:
      caml_failwith ((char *)__FUNCTION__);
    }
    Store_field (v, 1, v2);
  }
  CAMLreturn (rv);
#else
  not_supported ("virDomainGetSchedulerParameters");
#endif
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRDOMAINSETSCHEDULERPARAMETERS
extern int virDomainSetSchedulerParameters (virDomainPtr domain,
					    virSchedParameterPtr params,
					    int nparams)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_domain_set_scheduler_parameters (value domv, value paramsv)
{
#ifdef HAVE_VIRDOMAINSETSCHEDULERPARAMETERS
  CAMLparam2 (domv, paramsv);
  CAMLlocal1 (v);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  int nparams = Wosize_val (paramsv);
  virSchedParameter params[nparams];
  int r, i;
  char *name;

  for (i = 0; i < nparams; ++i) {
    v = Field (paramsv, i);	/* Points to the two-element tuple. */
    name = String_val (Field (v, 0));
    strncpy (params[i].field, name, VIR_DOMAIN_SCHED_FIELD_LENGTH);
    params[i].field[VIR_DOMAIN_SCHED_FIELD_LENGTH-1] = '\0';
    v = Field (v, 1);		/* Points to the sched_param_value block. */
    switch (Tag_val (v)) {
    case 0:
      params[i].type = VIR_DOMAIN_SCHED_FIELD_INT;
      params[i].value.i = Int32_val (Field (v, 0));
      break;
    case 1:
      params[i].type = VIR_DOMAIN_SCHED_FIELD_UINT;
      params[i].value.ui = Int32_val (Field (v, 0));
      break;
    case 2:
      params[i].type = VIR_DOMAIN_SCHED_FIELD_LLONG;
      params[i].value.l = Int64_val (Field (v, 0));
      break;
    case 3:
      params[i].type = VIR_DOMAIN_SCHED_FIELD_ULLONG;
      params[i].value.ul = Int64_val (Field (v, 0));
      break;
    case 4:
      params[i].type = VIR_DOMAIN_SCHED_FIELD_DOUBLE;
      params[i].value.d = Double_val (Field (v, 0));
      break;
    case 5:
      params[i].type = VIR_DOMAIN_SCHED_FIELD_BOOLEAN;
      params[i].value.b = Int_val (Field (v, 0));
      break;
    default:
      caml_failwith ((char *)__FUNCTION__);
    }
  }

  WEAK_SYMBOL_CHECK (virDomainSetSchedulerParameters);
  NONBLOCKING (r = virDomainSetSchedulerParameters (dom, params, nparams));
  CHECK_ERROR (r == -1, conn, "virDomainSetSchedulerParameters");

  CAMLreturn (Val_unit);
#else
  not_supported ("virDomainSetSchedulerParameters");
#endif
}

CAMLprim value
ocaml_libvirt_domain_set_vcpus (value domv, value nvcpusv)
{
  CAMLparam2 (domv, nvcpusv);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  int r, nvcpus = Int_val (nvcpusv);

  NONBLOCKING (r = virDomainSetVcpus (dom, nvcpus));
  CHECK_ERROR (r == -1, conn, "virDomainSetVcpus");

  CAMLreturn (Val_unit);
}

CAMLprim value
ocaml_libvirt_domain_pin_vcpu (value domv, value vcpuv, value cpumapv)
{
  CAMLparam3 (domv, vcpuv, cpumapv);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  int maplen = caml_string_length (cpumapv);
  unsigned char *cpumap = (unsigned char *) String_val (cpumapv);
  int vcpu = Int_val (vcpuv);
  int r;

  NONBLOCKING (r = virDomainPinVcpu (dom, vcpu, cpumap, maplen));
  CHECK_ERROR (r == -1, conn, "virDomainPinVcpu");

  CAMLreturn (Val_unit);
}

CAMLprim value
ocaml_libvirt_domain_get_vcpus (value domv, value maxinfov, value maplenv)
{
  CAMLparam3 (domv, maxinfov, maplenv);
  CAMLlocal5 (rv, infov, strv, v, v2);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  int maxinfo = Int_val (maxinfov);
  int maplen = Int_val (maplenv);
  virVcpuInfo info[maxinfo];
  unsigned char cpumaps[maxinfo * maplen];
  int r, i;

  memset (info, 0, sizeof (virVcpuInfo) * maxinfo);
  memset (cpumaps, 0, maxinfo * maplen);

  NONBLOCKING (r = virDomainGetVcpus (dom, info, maxinfo, cpumaps, maplen));
  CHECK_ERROR (r == -1, conn, "virDomainPinVcpu");

  /* Copy the virVcpuInfo structures. */
  infov = caml_alloc (maxinfo, 0);
  for (i = 0; i < maxinfo; ++i) {
    v2 = caml_alloc (4, 0); Store_field (infov, i, v2);
    Store_field (v2, 0, Val_int (info[i].number));
    Store_field (v2, 1, Val_int (info[i].state));
    v = caml_copy_int64 (info[i].cpuTime); Store_field (v2, 2, v);
    Store_field (v2, 3, Val_int (info[i].cpu));
  }

  /* Copy the bitmap. */
  strv = caml_alloc_string (maxinfo * maplen);
  memcpy (String_val (strv), cpumaps, maxinfo * maplen);

  /* Allocate the tuple and return it. */
  rv = caml_alloc_tuple (3);
  Store_field (rv, 0, Val_int (r)); /* number of CPUs. */
  Store_field (rv, 1, infov);
  Store_field (rv, 2, strv);

  CAMLreturn (rv);
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRDOMAINMIGRATE
extern virDomainPtr virDomainMigrate (virDomainPtr domain, virConnectPtr dconn,
				      unsigned long flags, const char *dname,
				      const char *uri, unsigned long bandwidth)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_domain_migrate_native (value domv, value dconnv, value flagsv, value optdnamev, value opturiv, value optbandwidthv, value unitv)
{
#ifdef HAVE_VIRDOMAINMIGRATE
  CAMLparam5 (domv, dconnv, flagsv, optdnamev, opturiv);
  CAMLxparam2 (optbandwidthv, unitv);
  CAMLlocal2 (flagv, rv);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  virConnectPtr dconn = Connect_val (dconnv);
  int flags = 0;
  const char *dname = Optstring_val (optdnamev);
  const char *uri = Optstring_val (opturiv);
  unsigned long bandwidth;
  virDomainPtr r;

  /* Iterate over the list of flags. */
  for (; flagsv != Val_int (0); flagsv = Field (flagsv, 1))
    {
      flagv = Field (flagsv, 0);
      if (flagv == Int_val(0))
	flags |= VIR_MIGRATE_LIVE;
    }

  if (optbandwidthv == Val_int (0)) /* None */
    bandwidth = 0;
  else				/* Some bandwidth */
    bandwidth = Int_val (Field (optbandwidthv, 0));

  WEAK_SYMBOL_CHECK (virDomainMigrate);
  NONBLOCKING (r = virDomainMigrate (dom, dconn, flags, dname, uri, bandwidth));
  CHECK_ERROR (!r, conn, "virDomainMigrate");

  rv = Val_domain (r, dconnv);

  CAMLreturn (rv);

#else /* virDomainMigrate not supported */
  not_supported ("virDomainMigrate");
#endif
}

CAMLprim value
ocaml_libvirt_domain_migrate_bytecode (value *argv, int argn)
{
  return ocaml_libvirt_domain_migrate_native (argv[0], argv[1], argv[2],
					      argv[3], argv[4], argv[5],
					      argv[6]);
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRDOMAINBLOCKSTATS
extern int virDomainBlockStats (virDomainPtr dom,
				const char *path,
				virDomainBlockStatsPtr stats,
				size_t size)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_domain_block_stats (value domv, value pathv)
{
#if HAVE_VIRDOMAINBLOCKSTATS
  CAMLparam2 (domv, pathv);
  CAMLlocal2 (rv,v);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  char *path = String_val (pathv);
  struct _virDomainBlockStats stats;
  int r;

  WEAK_SYMBOL_CHECK (virDomainBlockStats);
  NONBLOCKING (r = virDomainBlockStats (dom, path, &stats, sizeof stats));
  CHECK_ERROR (r == -1, conn, "virDomainBlockStats");

  rv = caml_alloc (5, 0);
  v = caml_copy_int64 (stats.rd_req); Store_field (rv, 0, v);
  v = caml_copy_int64 (stats.rd_bytes); Store_field (rv, 1, v);
  v = caml_copy_int64 (stats.wr_req); Store_field (rv, 2, v);
  v = caml_copy_int64 (stats.wr_bytes); Store_field (rv, 3, v);
  v = caml_copy_int64 (stats.errs); Store_field (rv, 4, v);

  CAMLreturn (rv);
#else
  not_supported ("virDomainBlockStats");
#endif
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRDOMAININTERFACESTATS
extern int virDomainInterfaceStats (virDomainPtr dom,
				    const char *path,
				    virDomainInterfaceStatsPtr stats,
				    size_t size)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_domain_interface_stats (value domv, value pathv)
{
#if HAVE_VIRDOMAININTERFACESTATS
  CAMLparam2 (domv, pathv);
  CAMLlocal2 (rv,v);
  virDomainPtr dom = Domain_val (domv);
  virConnectPtr conn = Connect_domv (domv);
  char *path = String_val (pathv);
  struct _virDomainInterfaceStats stats;
  int r;

  WEAK_SYMBOL_CHECK (virDomainInterfaceStats);
  NONBLOCKING (r = virDomainInterfaceStats (dom, path, &stats, sizeof stats));
  CHECK_ERROR (r == -1, conn, "virDomainInterfaceStats");

  rv = caml_alloc (8, 0);
  v = caml_copy_int64 (stats.rx_bytes); Store_field (rv, 0, v);
  v = caml_copy_int64 (stats.rx_packets); Store_field (rv, 1, v);
  v = caml_copy_int64 (stats.rx_errs); Store_field (rv, 2, v);
  v = caml_copy_int64 (stats.rx_drop); Store_field (rv, 3, v);
  v = caml_copy_int64 (stats.tx_bytes); Store_field (rv, 4, v);
  v = caml_copy_int64 (stats.tx_packets); Store_field (rv, 5, v);
  v = caml_copy_int64 (stats.tx_errs); Store_field (rv, 6, v);
  v = caml_copy_int64 (stats.tx_drop); Store_field (rv, 7, v);

  CAMLreturn (rv);
#else
  not_supported ("virDomainInterfaceStats");
#endif
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRSTORAGEPOOLGETINFO
extern int virStoragePoolGetInfo(virStoragePoolPtr pool, virStoragePoolInfoPtr info)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_storage_pool_get_info (value poolv)
{
#if HAVE_VIRSTORAGEPOOLGETINFO
  CAMLparam1 (poolv);
  CAMLlocal2 (rv, v);
  virStoragePoolPtr pool = Pool_val (poolv);
  virConnectPtr conn = Connect_polv (poolv);
  virStoragePoolInfo info;
  int r;

  WEAK_SYMBOL_CHECK (virStoragePoolGetInfo);
  NONBLOCKING (r = virStoragePoolGetInfo (pool, &info));
  CHECK_ERROR (r == -1, conn, "virStoragePoolGetInfo");

  rv = caml_alloc (4, 0);
  Store_field (rv, 0, Val_int (info.state));
  v = caml_copy_int64 (info.capacity); Store_field (rv, 1, v);
  v = caml_copy_int64 (info.allocation); Store_field (rv, 2, v);
  v = caml_copy_int64 (info.available); Store_field (rv, 3, v);

  CAMLreturn (rv);
#else
  not_supported ("virStoragePoolGetInfo");
#endif
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRSTORAGEVOLGETINFO
extern int virStorageVolGetInfo(virStorageVolPtr vol, virStorageVolInfoPtr info)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_storage_vol_get_info (value volv)
{
#if HAVE_VIRSTORAGEVOLGETINFO
  CAMLparam1 (volv);
  CAMLlocal2 (rv, v);
  virStorageVolPtr vol = Volume_val (volv);
  virConnectPtr conn = Connect_volv (volv);
  virStorageVolInfo info;
  int r;

  WEAK_SYMBOL_CHECK (virStorageVolGetInfo);
  NONBLOCKING (r = virStorageVolGetInfo (vol, &info));
  CHECK_ERROR (r == -1, conn, "virStorageVolGetInfo");

  rv = caml_alloc (3, 0);
  Store_field (rv, 0, Val_int (info.type));
  v = caml_copy_int64 (info.capacity); Store_field (rv, 1, v);
  v = caml_copy_int64 (info.allocation); Store_field (rv, 1, v);

  CAMLreturn (rv);
#else
  not_supported ("virStorageVolGetInfo");
#endif
}

#ifdef HAVE_WEAK_SYMBOLS
#ifdef HAVE_VIRJOBGETINFO
extern int virJobGetInfo(virJobPtr job, virJobInfoPtr info)
  __attribute__((weak));
#endif
#endif

CAMLprim value
ocaml_libvirt_job_get_info (value jobv)
{
#if HAVE_VIRJOBGETINFO
  CAMLparam1 (jobv);
  CAMLlocal1 (rv);
  virJobPtr job = Job_val (jobv);
  virConnectPtr conn = Connect_jobv (jobv);
  virJobInfo info;
  int r;

  WEAK_SYMBOL_CHECK (virJobGetInfo);
  NONBLOCKING (r = virJobGetInfo (job, &info));
  CHECK_ERROR (r == -1, conn, "virJobGetInfo");

  rv = caml_alloc (5, 0);
  Store_field (rv, 0, Val_int (info.type));
  Store_field (rv, 1, Val_int (info.state));
  Store_field (rv, 2, Val_int (info.runningTime));
  Store_field (rv, 3, Val_int (info.remainingTime));
  Store_field (rv, 4, Val_int (info.percentComplete));

  CAMLreturn (rv);
#else
  not_supported ("virJobGetInfo");
#endif
}

/*----------------------------------------------------------------------*/

CAMLprim value
ocaml_libvirt_virterror_get_last_error (value unitv)
{
  CAMLparam1 (unitv);
  CAMLlocal1 (rv);
  virErrorPtr err = virGetLastError ();

  rv = Val_opt (err, (Val_ptr_t) Val_virterror);

  CAMLreturn (rv);
}

CAMLprim value
ocaml_libvirt_virterror_get_last_conn_error (value connv)
{
  CAMLparam1 (connv);
  CAMLlocal1 (rv);
  virConnectPtr conn = Connect_val (connv);

  rv = Val_opt (conn, (Val_ptr_t) Val_connect);

  CAMLreturn (rv);
}

CAMLprim value
ocaml_libvirt_virterror_reset_last_error (value unitv)
{
  CAMLparam1 (unitv);
  virResetLastError ();
  CAMLreturn (Val_unit);
}

CAMLprim value
ocaml_libvirt_virterror_reset_last_conn_error (value connv)
{
  CAMLparam1 (connv);
  virConnectPtr conn = Connect_val (connv);
  virConnResetLastError (conn);
  CAMLreturn (Val_unit);
}

/*----------------------------------------------------------------------*/

/* Initialise the library. */
CAMLprim value
ocaml_libvirt_init (value unit)
{
  CAMLparam1 (unit);
  CAMLlocal1 (rv);
  int r;

  r = virInitialize ();
  CHECK_ERROR (r == -1, NULL, "virInitialize");

  CAMLreturn (Val_unit);
}