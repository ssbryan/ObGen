/*
Name:		Stephen Bryan
Project: 	Asteroid Accretion
Term:		Fall 2017
Advisor:	Dr Imamura
*/

let objects = [];
/*
init() is the basic method in the script, called after having loaded the HTML.
All functionality for the project originates here
*/
function init()
{
	let createInit = document.getElementById('createText');
    createInit.addEventListener("click", handleCreateButtonClick);
}

function handleCreateButtonClick()
{
    // make sure the array is empty
    while (objects.length)
    {
        objects.pop();
    }
    
    // number of objects
	let num = document.getElementById('numberObj');
    
    if (num.value.length <= 0)
    {
        alert("Please enter a number of objects");
        return;
    }

    let numObj = num.value;
    
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // mass quantities
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // min mass
	let minM = document.getElementById('minMass');
    
    if (minM.value.length <= 0)
    {
        alert("Please enter a minimum mass");
        return;
    }

    let minMass = parseFloat(minM.value);

    // max mass
	let maxM = document.getElementById('maxMass');
    
    if (maxM.value.length <= 0)
    {
        alert("Please enter a maximum mass");
        return;
    }

    let maxMass = parseFloat(maxM.value);

    // selected distribution
    let distM = document.getElementById("massDist").value;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Velocity/Momentum quantities
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // min vel/mom
	let minVP = document.getElementById('minVP');
    
    if (minVP.value.length <= 0)
    {
        alert("Please enter a minimum for velocity or momentum");
        return;
    }

    let minVelMom = parseFloat(minVP.value);

    // max vel/mom
	let maxVP = document.getElementById('maxVP');
    
    if (maxVP.value.length <= 0)
    {
        alert("Please enter a maximum velocity or momentum");
        return;
    }

    let maxVelMom = parseFloat(maxVP.value);

    // selected distribution
    let distVP = document.getElementById("vpDist").value;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Environment quantities
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // min mass
	let spanX = document.getElementById('spanD');
    
    if (spanX.value.length <= 0)
    {
        alert("Please enter a value for the span of the volume");
        return;
    }

    let spanSpace = parseFloat(spanX.value);

    // selected distribution
    let distObj = document.getElementById("objDist").value;

///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Options
///////////////////////////////////////////////////////////////////////////////////////////////////////////

    let optStr = "";
	let momTol = document.getElementById('momtolOpt');
    
    if (momTol.value.length > 0)
    {
        optStr += "momtol = ";
        optStr += momTol.value;
    }

	let dvnormTol = document.getElementById('dvnormtolOpt');
    
    if (dvnormTol.value.length > 0)
    {
        if (optStr.length > 0)
        {
            optStr +=  ", ";
        }
        
        optStr += "dvnormtol = ";
        optStr += dvnormTol.value;
    }

	let runsOpt = document.getElementById('runsOpt');
    
    if (runsOpt.value.length > 0)
    {
        if (optStr.length > 0)
        {
            optStr +=  ", ";
        }
        
        optStr += "runs = ";
        optStr += runsOpt.value;
    }

	let tstepOpt = document.getElementById('tstepOpt');
    
    if (tstepOpt.value.length > 0)
    {
        if (optStr.length > 0)
        {
            optStr +=  ", ";
        }
        
        optStr += "tstep = ";
        optStr += tstepOpt.value;
    }

	let minvOpt = document.getElementById('minvforcheckOpt');
    let  minv = 0.001;
    
    if (minvOpt.value.length > 0)
    {
        if (optStr.length > 0)
        {
            optStr +=  ", ";
        }
        
        optStr += "minvforcheck = ";
        optStr += minvOpt.value;
    }
    
///////////////////////////////////////////////////////////////////////////////////////////////////////////
    // Write text
///////////////////////////////////////////////////////////////////////////////////////////////////////////
	let outText = document.getElementById('initText');
    let outStr = "Options: " + optStr;
    outText.innerHTML= outStr;
    let nn = parseInt(numObj);
    
    // get statistics for loc, vel
    let avgx = 0.0;
    let avgy = 0.0;
    let avgz = 0.0;
    let avgvx = 0.0;
    let avgvy = 0.0;
    let avgvz = 0.0;
    
    // get each object set up based on user input
    // and create string
    for (let i = 0; i < nn; i++)
    {
        // create objects
        let m = getMass(minMass, maxMass, distM);
        let v = getV(m, minVelMom, maxVelMom, distVP);
        let loc = getLoc(spanSpace, distObj);
        let obj = {x: loc.X, y: loc.Y, z: loc.Z, vx: v.X, vy: v.Y, vz: v.Z, mass: m};
        objects.push(obj);
        let strObj = "\n" + loc.X + " " + loc.Y + " " + loc.Z + " " + v.X + " " + v.Y + " " + v.Z + " " + m;
        outStr += strObj;
        
        avgx += loc.X;
        avgy += loc.Y;
        avgz += loc.Z;
        avgvx += v.X;
        avgvy += v.Y;
        avgvz += v.Z;
    }

    avgx /= nn;
    avgy /= nn;
    avgz /= nn;
    avgvx /= nn;
    avgvy /= nn;
    avgvz /= nn;
    
    // analyze input data to get 
    // --total momentum
    // --total angular momentum
    // --CoM compared to 0,0,0
    // --total kinetic energy
    
    // first, get CoM and total momentum
    let xc = 0.0;
    let yc = 0.0;
    let zc = 0.0;
    let totmass = 0.0;
    let totmomx = 0.0;
    let totmomy = 0.0;
    let totmomz = 0.0;
    
    for (let i = 0; i < nn; i++)
    {
        let obj = objects[i];
        let m = parseFloat(obj.mass);
        xc += m * obj.x;
        yc += m * obj.y;
        zc += m * obj.z;
        totmass += m;
        
        totmomx += m * obj.vx;
        totmomy += m * obj.vy;
        totmomz += m * obj.vz;
    }
    
	let avgData = document.getElementById('avgdata');
    let avgdataStr = "Average location: " + avgx + ", " + avgy + ", " + avgz + " Average velocity: " + avgvx + ", " + avgvy + ", " + avgvz + " Total mass: " + totmass + " Average mass: " + totmass / nn + " Average random: " + rnum / numr;
    avgData.innerHTML = avgdataStr;
    
    xc /= totmass;
    yc /= totmass;
    zc /= totmass;
    
    // now get angular momentum
    let angmomx = 0.0;
    let angmomy = 0.0;
    let angmomz = 0.0;
    
    for (let i = 0; i < nn; i++)
    {
        let obj = objects[i];
        let dist2 = (obj.x - xc) * (obj.x - xc) + (obj.y - yc) * (obj.y - yc) + (obj.z - zc) * (obj.z - zc);
        let dist = Math.sqrt(dist2);
        let dot = obj.vx * obj.x + obj.vy * obj.y + obj.vz * obj.z;
        angmomx += obj.mass * (obj.vx * dist - (obj.vx - xc) * dot / dist);
        angmomy += obj.mass * (obj.vy * dist - (obj.vy - xc) * dot / dist);
        angmomz += obj.mass * (obj.vz * dist - (obj.vz - xc) * dot / dist);
    }
    
	let cmData = document.getElementById('cmdata');
    let cmdataStr = "Center of Mass: " + xc + ", " + yc + ", " + zc;
    cmData.innerHTML = cmdataStr;
	let momData = document.getElementById('momdata');
    let momdataStr = "Momentum: " + totmomx  + ", " + totmomy + ", " + totmomz;
    momData.innerHTML = momdataStr;
	let angmomData = document.getElementById('angmomdata');
    let angmomdataStr = "Ang Momentum: " + angmomx  + ", " + angmomy + ", " + angmomz;
    angmomData.innerHTML = angmomdataStr;
    
    outText.innerHTML= outStr;
}

function getMass(minMass, maxMass, distM)
{
    let m = (minMass + maxMass) / 2;
    let mn2 = Math.sqrt(minMass);
    let mx2 = Math.sqrt(maxMass);
    let mn4 = Math.sqrt(mn2);
    let mx4 = Math.sqrt(mx2);
    
    if (distM == "smallNum")
    {
        m = getRandomNumber(mn2, mx2);
        m *= m;
    }
    else if (distM == "uniform")
    {
        m = getRandomNumber(minMass, maxMass);
    }
    else if (distM == "smallerNum")
    {
        m = getRandomNumber(mn4, mx4);
        m *= m;
        m *= m;
    }
    
    return m;
}

function getV(m, minVP, maxVP, distvp)
{
    let minv = parseInt(minVP);
    let maxv = parseInt(maxVP);
    // need negatives, too
    // double the range, then check whether the resulting random number is in the upper half 
    // and make it negative, if so
    let diff = maxv - minv;
    let vx = getRandomNumber(minv, maxv + diff);
    
    if (vx >= maxv)
    {
        vx -= diff;
        vx = -vx;
    }
    
    let vy = getRandomNumber(minv, maxv + diff);
    
    if (vy >= maxv)
    {
        vy -= diff;
        vy = -vy;
    }
    
    let vz = getRandomNumber(minv, maxv + diff);
    
    if (vz >= maxv)
    {
        vz -= diff;
        vz = -vz;
    }
    
    let v = {X: vx, Y: vy, Z: vz};
    return v;
}

function getLoc(spanX, distObj)
{
    let x = getRandomNumber(-spanX / 2, spanX / 2);
    let y = getRandomNumber(-spanX / 2, spanX / 2);
    let z = getRandomNumber(-spanX / 2, spanX / 2);
    let loc = {X: x, Y: y, Z: z};
    return loc;
}

var rnum = 0.0;
var numr = 0;
function getRandomNumber(min, max) 
{
    let r = Math.random();
    rnum += r;
    numr++;
    return (r * (max - min) + min);
}

// After DOM loaded, initiate process to retreive current location
// startup after HTML load
window.onload = init;