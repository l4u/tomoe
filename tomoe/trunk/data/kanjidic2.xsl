<?xml version="1.0" encoding="UTF-8"?>
<xsl:stylesheet version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method = "xml" indent="yes" encoding="UTF-8" doctype-system = "tomoe-dict.dtd" standalone="no"/>
<xsl:template match="/">
  <tomoe_dictionary name="Jim Breen's KanjiDic2" meta="meta.xsl">
    <xsl:apply-templates select="//character"/>
  </tomoe_dictionary>
</xsl:template>
<xsl:template match="character">
  <character>
    <xsl:apply-templates select="literal"/>
    <xsl:apply-templates select="reading_meaning" mode="tomoe"/>
    <meta>
      <xsl:apply-templates select="misc|codepoint"/>
      <meaning>
        <xsl:apply-templates select="reading_meaning" mode="meta"/>
      </meaning>
    </meta>
  </character>
</xsl:template>
<xsl:template match="literal">
  <literal>
    <xsl:value-of select="."/>
  </literal>
</xsl:template>

<xsl:template match="reading_meaning" mode="tomoe">
  <xsl:apply-templates select="rmgroup" mode="tomoe"/>
</xsl:template>
<xsl:template match="rmgroup" mode="tomoe">
  <readings>
    <xsl:apply-templates select="reading[@r_type='ja_on']|reading[@r_type='ja_kun']"/>
  </readings>
</xsl:template>
<xsl:template match="reading">
  <r>
    <xsl:value-of select="."/>
  </r>
</xsl:template>

<xsl:template match="misc">
  <xsl:apply-templates select="grade"/>
</xsl:template>
<xsl:template match="grade">
  <jouyou>
    <xsl:value-of select="."/>
  </jouyou>
</xsl:template>

<xsl:template match="codepoint">
  <xsl:apply-templates select="cp_value[@cp_type='jis208']" mode="jis208"/>
  <xsl:apply-templates select="cp_value[@cp_type='jis212']" mode="jis212"/>
  <xsl:apply-templates select="cp_value[@cp_type='jis213']" mode="jis213"/>
  <xsl:apply-templates select="cp_value[@cp_type='ucs']" mode="ucs"/>
</xsl:template>
<xsl:template match="cp_value" mode="jis208">
  <jis208>
    <xsl:value-of select="."/>
  </jis208>
</xsl:template>
<xsl:template match="cp_value" mode="jis212">
  <jis212>
    <xsl:value-of select="."/>
  </jis212>
</xsl:template>
<xsl:template match="cp_value" mode="jis213">
  <jis213>
    <xsl:value-of select="."/>
  </jis213>
</xsl:template>
<xsl:template match="cp_value" mode="ucs">
  <ucs>
    <xsl:value-of select="."/>
  </ucs>
</xsl:template>

<xsl:template match="reading_meaning" mode="meta">
  <xsl:apply-templates select="rmgroup" mode="meta"/>
</xsl:template>
<xsl:template match="rmgroup" mode="meta">
  <meaning>
    <xsl:apply-templates select="meaning[@m_lang='en']"/>
    <xsl:apply-templates select="meaning[not(@m_lang)]"/>
  </meaning>
</xsl:template>
<xsl:template match="meaning">
  <xsl:value-of select="."/>
  <xsl:if test="not(position()=last())"><xsl:text>, </xsl:text></xsl:if>
</xsl:template>

</xsl:stylesheet>
