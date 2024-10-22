

local opencv_lua = require("init")
local cv = opencv_lua.cv
local INDEX_BASE = 1
local deviceWidth = 1080  -- 从 adb shell wm size 获取
local deviceHeight = 1920  -- 从 adb shell wm size 获取

local function getImageSize(image)
    local size = cv.GetSize(image)
    return {width = size.width, height = size.height}
end

local function match()
    local src = cv.imread(cv.samples.findFile("src_image.png"))
    local templ = cv.imread(cv.samples.findFile("Identified_image.png"))
    if src:empty() then
        print("src is empty input")
        return nil
    else
        print("src_size:", src.width, src.height)
    end

    if templ:empty() then
        print("templ is empty input")
        return nil
    end

    local scaleX = deviceWidth/src.width
    local scaleY = deviceHeight/src.height
    local result = cv.matchTemplate(src, templ, cv.TM_CCOEFF)
    cv.imshow("result1", result)

    local minVal, maxVal, minLoc, maxLoc = cv.minMaxLoc(result)
    local matchLoc = maxLoc
    
    cv.rectangle(
        src,
        matchLoc,
        { matchLoc[0 + INDEX_BASE] + templ.width, matchLoc[1 + INDEX_BASE] + templ.height },
        2
    )
    local center = {x = matchLoc[0 + INDEX_BASE] + math.floor(templ.width / 2), y = matchLoc[1 + INDEX_BASE] + math.floor(templ.height / 2)}
    local point = {x =matchLoc[0 + INDEX_BASE], y = matchLoc[1 + INDEX_BASE]}
    cv.circle(
        src,
        {center.x, center.y},
        5,
        {0, 0, 255},
        2
    )
    print("Center: ", math.floor(center.x*scaleX), math.floor(center.y*scaleY))
    print("Point: ", point.x*scaleX, point.y*scaleY)

    cv.imshow("src", src)
    cv.waitKey(0)
end

match()