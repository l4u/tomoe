<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="text"/>

<xsl:template match="/">
<xsl:text></xsl:text>
<xsl:apply-templates select="node()"/>
</xsl:template>

<xsl:template match="ch">
<xsl:text></xsl:text>
<xsl:apply-templates select="meta"/>
</xsl:template>

<xsl:template match="meta">
<xsl:text></xsl:text>
<xsl:apply-templates select="desc"/>
<xsl:apply-templates select="meaning"/>
<xsl:apply-templates select="jis208"/>
<xsl:apply-templates select="jis212"/>
<xsl:apply-templates select="jis213"/>
<xsl:apply-templates select="ucs"/>
<xsl:apply-templates select="jouyou"/>
<xsl:apply-templates select="jlpt"/>
</xsl:template>

<xsl:template match="desc">
<xsl:text>Description: </xsl:text>
<xsl:value-of select="."/>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="meaning">
<xsl:if test="not(meaning[1]='')">
  <xsl:text>Meaning: </xsl:text>
  <xsl:value-of select="."/>
  <xsl:text>
</xsl:text>
</xsl:if>
</xsl:template>

<xsl:template match="jis208">
<xsl:text>JIS X 208: </xsl:text>
<xsl:value-of select="."/>
<xsl:text>
</xsl:text>
</xsl:template>
<xsl:template match="jis212">
<xsl:text>JIS X 212: </xsl:text>
<xsl:value-of select="."/>
<xsl:text>
</xsl:text>
</xsl:template>
<xsl:template match="jis213">
<xsl:text>JIS X 213: </xsl:text>
<xsl:value-of select="."/>
<xsl:text>
</xsl:text>
</xsl:template>
<xsl:template match="ucs">
<xsl:text>Unicode: </xsl:text>
<xsl:value-of select="."/>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="jouyou">
<xsl:text>常用: </xsl:text>
<xsl:value-of select="."/>
<xsl:text>
</xsl:text>
</xsl:template>

<xsl:template match="jlpt">
<xsl:text>JLPT Level: </xsl:text>
<xsl:value-of select="."/>
<xsl:text>
</xsl:text>
</xsl:template>

</xsl:stylesheet>