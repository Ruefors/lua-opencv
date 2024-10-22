package.path = arg[0]:gsub("[^/\\]+%.lua", '?.lua;'):gsub('/', package.config:sub(1, 1)) .. package.path

local opencv_lua = require("init")
local cv = opencv_lua.cv
local INDEX_BASE = 1
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
    end

    if templ:empty() then
        print("templ is empty input")
        return nil
    end

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
    cv.imshow("src", src)
    cv.waitKey(0)
end

match()