#pragma once

#define COMMON_NAMESPACE common
#define COMMON_NAMESPACE_FULL COMMON_NAMESPACE
#define COMMON_NAMESPACE_BEGIN namespace COMMON_NAMESPACE_FULL{
#define COMMON_NAMESPACE_END }

COMMON_NAMESPACE_BEGIN
COMMON_NAMESPACE_END

#define CLIENT_NAMESPACE client
#define CLIENT_NAMESPACE_FULL CLIENT_NAMESPACE
#define CLIENT_NAMESPACE_BEGIN namespace CLIENT_NAMESPACE_FULL{
#define CLIENT_NAMESPACE_END }

CLIENT_NAMESPACE_BEGIN
using namespace ::COMMON_NAMESPACE_FULL;
CLIENT_NAMESPACE_END

#define SERVER_NAMESPACE server
#define SERVER_NAMESPACE_FULL SERVER_NAMESPACE
#define SERVER_NAMESPACE_BEGIN namespace SERVER_NAMESPACE_FULL{
#define SERVER_NAMESPACE_END }

SERVER_NAMESPACE_BEGIN
using namespace ::COMMON_NAMESPACE_FULL;
SERVER_NAMESPACE_END

#define GUI_NAMESPACE gui
#define GUI_NAMESPACE_FULL GUI_NAMESPACE
#define GUI_NAMESPACE_BEGIN namespace GUI_NAMESPACE{
#define GUI_NAMESPACE_END }

#define LOGIC_NAMESPACE logic
#define LOGIC_NAMESPACE_FULL LOGIC_NAMESPACE
#define LOGIC_NAMESPACE_BEGIN namespace LOGIC_NAMESPACE{
#define LOGIC_NAMESPACE_END }

#define DATA_NAMESPACE data
#define DATA_NAMESPACE_FULL DATA_NAMESPACE
#define DATA_NAMESPACE_BEGIN namespace DATA_NAMESPACE{
#define DATA_NAMESPACE_END }

#define MESSAGES_NAMESPACE messages
#define MESSAGES_NAMESPACE_FULL DATA_NAMESPACE_FULL::MESSAGES_NAMESPACE
#define MESSAGES_NAMESPACE_BEGIN namespace MESSAGES_NAMESPACE{
#define MESSAGES_NAMESPACE_END }

#define NETWORKING_NAMESPACE networking
#define NETWORKING_NAMESPACE_FULL NETWORKING_NAMESPACE
#define NETWORKING_NAMESPACE_BEGIN namespace NETWORKING_NAMESPACE{
#define NETWORKING_NAMESPACE_END }