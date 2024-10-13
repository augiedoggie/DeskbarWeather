
execute_process(
	COMMAND finddir B_SYSTEM_HEADERS_DIRECTORY
	OUTPUT_VARIABLE FINDDIR_OUTPUT
	OUTPUT_STRIP_TRAILING_WHITESPACE
)

set(B_SYSTEM_HEADERS_DIRECTORY ${FINDDIR_OUTPUT} CACHE PATH "")

include_directories(
	"${B_SYSTEM_HEADERS_DIRECTORY}/private"
	"${B_SYSTEM_HEADERS_DIRECTORY}/private/interface"
	"${B_SYSTEM_HEADERS_DIRECTORY}/private/shared"
	"${B_SYSTEM_HEADERS_DIRECTORY}/private/netservices"
)

haiku_add_executable(DeskbarWeather
	DeskbarWeather.rdef
	BitmapView.cpp
	Condition.cpp
	DeskbarWeatherApp.cpp
	DeskbarWeatherView.cpp
	ForecastWindow.cpp
	IpApiLocationProvider.cpp
	JsonRequest.cpp
	OpenMeteo.cpp
	SettingsWindow.cpp
	WeatherSettings.cpp
)

target_link_libraries(DeskbarWeather be netservices bnetapi shared)
