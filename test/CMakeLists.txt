SET(TEST_SOURCES
  test.C
  auth/BCryptTest.C
  auth/SHA1Test.C
  chart/WChartTest.C
  json/JsonParserTest.C
  http/HttpClientTest.C
  mail/MailClientTest.C
  models/WBatchEditProxyModelTest.C
  models/WStandardItemModelTest.C
  private/HttpTest.C
  private/CExpressionParserTest.C
  private/I18n.C
  utf8/Utf8Test.C
  utf8/XmlTest.C
  utils/Base64Test.C
  wdatetime/WDateTimeTest.C
  length/WLengthTest.C
  color/WColorTest.C
  paintdevice/WSvgTest.C
  payment/MoneyTest.C
  locale/LocaleNumberTest.C
)

IF (WT_HAS_WRASTERIMAGE)
   SET(TEST_SOURCES ${TEST_SOURCES}
     paintdevice/WRasterTest.C
   )
ENDIF(WT_HAS_WRASTERIMAGE)

ADD_EXECUTABLE(test
  ${TEST_SOURCES}
)

TARGET_LINK_LIBRARIES(test wt wttest ${BOOST_FS_LIB})

# Test all dbo backends
SET(DBO_TEST_SOURCES
  test.C
  dbo/DboTest.C
  dbo/DboTest2.C
  dbo/DboTest3.C
  dbo/Benchmark.C
  private/DboImplTest.C
)

IF(HAVE_POSTGRES)
  ADD_EXECUTABLE(       test.postgres ${DBO_TEST_SOURCES})
  TARGET_LINK_LIBRARIES(test.postgres wt wttest wtdbo wtdbopostgres)
  SET_TARGET_PROPERTIES(test.postgres PROPERTIES COMPILE_FLAGS "-DPOSTGRES")
  MESSAGE("** Testing Wt::Dbo using Postgres backend")
ENDIF(HAVE_POSTGRES)

IF(HAVE_MYSQL)
  ADD_EXECUTABLE(       test.mysql ${DBO_TEST_SOURCES})
  TARGET_LINK_LIBRARIES(test.mysql wt wttest wtdbo wtdbomysql)
  SET_TARGET_PROPERTIES(test.mysql PROPERTIES COMPILE_FLAGS "-DMYSQL")
  MESSAGE("** Testing Wt::Dbo using Mysql backend")
ENDIF(HAVE_MYSQL)

IF(HAVE_FIREBIRD)
  ADD_EXECUTABLE(       test.firebird ${DBO_TEST_SOURCES})
  TARGET_LINK_LIBRARIES(test.firebird wt wttest wtdbo wtdbofirebird)
  SET_TARGET_PROPERTIES(test.firebird PROPERTIES COMPILE_FLAGS "-DFIREBIRD")
  MESSAGE("** Testing Wt::Dbo using Firebird backend")
ENDIF(HAVE_FIREBIRD)

IF(HAVE_SQLITE)
  ADD_EXECUTABLE(       test.sqlite3 ${DBO_TEST_SOURCES})
  TARGET_LINK_LIBRARIES(test.sqlite3 wt wttest wtdbo wtdbosqlite3)
  SET_TARGET_PROPERTIES(test.sqlite3 PROPERTIES COMPILE_FLAGS "-DSQLITE3")
  MESSAGE("** Testing Wt::Dbo using Sqlite3 backend")
ENDIF(HAVE_SQLITE)


INCLUDE_DIRECTORIES(${WT_SOURCE_DIR}/src)

IF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
  SUBDIRS(interactive)
ENDIF (EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/interactive)
