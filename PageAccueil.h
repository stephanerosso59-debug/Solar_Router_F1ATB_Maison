//************************************************
// Page principale HTML et Javascript 
//************************************************
const char *MainHtml = R"====(
<!doctype html>
<html>
<head>
<meta charset="UTF-8">
<link rel="stylesheet" href="commun.css">
<style>

body{
    margin:0 auto;        /* recentre le body (commun.css met margin:auto, il ne faut PAS le mettre a 0) */
    max-width:1200px;     /* remplace le max-width:1000px de commun.css */
    padding:20px;
    background:#0f172a;
    color:#e2e8f0;
    font-family:Segoe UI,Roboto,Arial,sans-serif;
    text-align:center;
}

.header{
    margin-bottom:20px;
}

.header h1{
    margin:0;
    color:white;
    font-size:2rem;
    font-weight:600;
}

#date{
    margin-top:8px;
    color:#94a3b8;
    font-size:1rem;
}

.ri{text-align:right;}
.ce{text-align:center;}
.centrer{text-align:center;}

.tableau{
    background:#1e293b;
    border-radius:4px;
    padding:20px;
    margin:10px auto;
    box-shadow:none;
    display:inline-block;
    max-width:1200px;
    width:95%;
}

table{
    width:100%;
    border-collapse:collapse;
    border:none;
}

th{
    background:#334155;
    color:white;
    padding:12px;
    font-weight:600;
}

td{
    padding:12px;
    border-bottom:1px solid #334155;
}

tr:hover{
    background:#243244;
}

.blue,
.Wh,
.W,
.VA,
.deg,
.temper{
    background:transparent;
}

svg{
    border:none;
    border-radius:4px;
    background:#1e293b;
    box-shadow:none;
    margin-top:15px;
}

#LED{
    position:fixed;
    top:15px;
    right:15px;
    width:12px;
    height:12px;
    border:none;
    border-radius:50%;
    background:#ef4444;
    box-shadow:none;
}

.Baccueil{
    border:none;
}

.jaugeBack{
    background:#334155;
    width:220px;
    height:28px;
    border-radius:14px;
    position:relative;
    overflow:hidden;
    margin:auto;
}

.jauge{
    background:linear-gradient(90deg,#22c55e,#84cc16);
    height:100%;
    position:absolute;
    left:0;
    top:0;
}

.w100{
    width:100%;
    position:absolute;
    left:0;
    top:4px;
    color:white;
    font-weight:600;
}

.dispT,
#SVG_PW48hT,
#SVG_PW2sT,
#SVG_Temp48h,
#SVG_Ouvertures{
    display:none;
}

#donneeDistante{
    font-size:0.85rem;
    color:#fbbf24;
    text-align:center;
    margin:15px;
    display:none;
}

#info{
    position:absolute;
    display:none;
    border-left:2px solid #60a5fa;
    pointer-events:none;
}

#info_txt{
    position:absolute;
    background:rgba(15,23,42,.95);
    color:white;
    padding:8px;
    right:10px;
    border-radius:8px;
    border:1px solid #475569;
    text-align:right;
}

#couleurTarif_jour,
#couleurTarif_J1{
    font-size:11px;
    font-weight:bold;
    border-radius:8px;
    color:black;
}

input[type=button]{
    border:none;
    border-radius:8px;
    padding:8px 16px;
    background:#2563eb;
    color:white;
    cursor:pointer;
    font-weight:600;
}

input[type=button]:hover{
    background:#1d4ed8;
}

svg text[onclick]{
    cursor:pointer;
}

.foot{
    color:#94a3b8;
    font-size:14px;
    margin-top:30px;
}

</style>

</head>

<body onload='Init();'>
<center>

<div id='LED'></div>

<div class="header">
    <h1>Routeur Solaire</h1>
    <div id='date'>DATE</div>
</div>

<div id='lesOnglets'></div>

<div>
<div class='tableau'>
<table>

<tr class='blue'>
<th></th>
<th colspan='2' id='nomSondeMobile'>Maison</th>
<th colspan='2' class='dispT' id='nomSondeFixe'>Fixe</th>
<th id='couleurTarif_jour'></th>
</tr>

<tr class='blue'>
<th></th>
<th>Soutirée</th>
<th>Injectée</th>
<th class='dispT'>Conso.</th>
<th class='dispT' id="produite">Produite</th>
<th id='couleurTarif_J1'></th>
</tr>

<tr class='W'>
<td>Puissance Active <small>(Pw)</small></td>
<td id='PwS_M' class='ri'></td>
<td id='PwI_M' class='ri'></td>
<td id='PwS_T' class='dispT ri'></td>
<td id='PwI_T' class='dispT ri'></td>
<td>W</td>
</tr>

<tr class='VA' id='ligneVA'>
<td>Puissance Apparente</td>
<td id='PVAS_M' class='ri'></td>
<td id='PVAI_M' class='ri'></td>
<td id='PVAS_T' class='dispT ri'></td>
<td id='PVAI_T' class='dispT ri'></td>
<td>VA</td>
</tr>

<tr class='Wh'>
<td>Energie Active du jour</td>
<td id='EAJS_M' class='ri'></td>
<td id='EAJI_M' class='ri'></td>
<td id='EAJS_T' class='dispT ri'></td>
<td id='EAJI_T' class='dispT ri'></td>
<td>Wh</td>
</tr>

<tr class='Wh'>
<td>Energie Active Totale</td>
<td id='EAS_M' class='ri'></td>
<td id='EAI_M' class='ri'></td>
<td id='EAS_T' class='dispT ri'></td>
<td id='EAI_T' class='dispT ri'></td>
<td>Wh</td>
</tr>

</table>
</div>
</div>

<div id="donneeDistante">Données distantes</div>

<div id='etatActions'></div>

<p id='SVG_PW2sM'></p>
<p id='SVG_PW2sT'></p>
<p id='SVG_PW48hM'></p>
<p id='SVG_PW48hT'></p>
<p id='SVG_Temp48h'></p>
<p id='SVG_Ouvertures'></p>
<p id='SVG_Wh1an'></p>

<div id='info'>
    <div id='info_txt'></div>
</div>

<div class='foot'>
    Données RMS
    <div id='source'></div>
</div>

<div id='pied'></div>

<script src='MainJS1'></script>
<script src='MainJS2'></script>
<script src='MainJS3'></script>
<script src='/ParaCommunJS'></script>
<script src='/CommunCouleurJS'></script>

</center>
</body>
</html>
)====";

