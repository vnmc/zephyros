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

/*
 *	HOW TO GENERATE A KEY PAIR
 * for use with Zephyros
 *
 *		openssl dsaparam -out dsaparam.pem 1024 (or use longer key if desired)
 *		openssl gendsa -out privkey.pem dsaparam.pem
 *		openssl dsa -in privkey.pem -outform PEM -pubout -out pubkey.pem
 *
 *  Make sure to copy the public key into Zephyros (LicenseManager->SetLicenseInfo(productCode, publicKey))
 */

// Generates a license
function generateLicense($seed, $privateKey){
	$binary_signature ="";
	openssl_sign($seed, $binary_signature, $privateKey, OPENSSL_ALGO_DSS1);

	// base 32 encode the stuff
	$encoded = base32_encode($binary_signature);

	// replace O with 8 and I with 9
	$replacement = str_replace("O", "8", str_replace("I", "9", $encoded));

	//remove padding if any.
	$padding = trim(str_replace("=", "", $replacement));

	$dashed = rtrim(chunk_split($padding, 5,"-"));
	$theKey = substr($dashed, 0 , strlen($dashed) -1);
	return $theKey;
}

// Verifies a license - can be used e.g. in the order fullfillment process
// to make sure the license is ok.
function verifyLicense($licenseKey, $seed, $publicKey){
	$replacement = str_replace("8", "O", str_replace("9", "I", $licenseKey));
	$undashed = trim(str_replace("-", "", $replacement));

	// Pad the output length to a multiple of 8 with '=' characters
	$desiredLength = strlen($undashed);
	if($desiredLength % 8 != 0) {
		$desiredLength += (8 - ($desiredLength % 8));
		$undashed = str_pad($undashed, $desiredLength, "=");
	}
	$decodedHash = base32_decode($undashed);
	//digest the original Data
	$ok = openssl_verify($seed, $decodedHash, $publicKey, OPENSSL_ALGO_DSS1);
	return $ok;
}

?>