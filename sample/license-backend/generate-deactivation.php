<?php
/*******************************************************************************
 * Copyright (c) 2015 Vanamco AG, http://www.vanamco.com
 *
 * The MIT License (MIT)
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 *
 * Contributors:
 * Matthias Christen, Vanamco AG
 * Florian Müller, Vanamco AG
 *******************************************************************************/

include('base32.php');
include('license.php');


// Deactivate a license for a specific computer


$publicKey = file_get_contents(realpath("publicKey.pem"));
$privateKey = file_get_contents(realpath("privateKey.pem"));
/* (See license.php for how to generate the keypair) */

$clientSideLicense = $_REQUEST["license"];
$eth0 = $_REQUEST["eth0"];
$productCode = $_REQUEST["productCode"];

// The license below is created for the seed "Mike May|S Corp|102" - in your code,
// fetch it from where you store your licenses.
$serverSideLicense = "GAWAE-FAFAD-C85YF-X8JGH-BL5GG-XJSSN-4EXDF-C6XQC-CRQ6R-8VTGT-N7XRL-WGLQP-VLFM7-D9NLJ-T5GM";

$licenseFound =  strcmp($clientSideLicense, $serverSideLicense) == 0;

if($licenseFound)
{
    $seed = $eth0;
    $activation = generateLicense($seed, $privateKey);
    header("Content-type: application/json");
    echo json_encode(array("status" => "success", "message" => "product deactivated", "deactivation" => $activation));
}
else
{
    header('Content-type: application/json');
    echo json_encode(array("status" => "failure", "message" => "no matching license found"));
}

?>